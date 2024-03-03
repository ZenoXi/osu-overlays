#include "CudaSmokeSim.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <memory>

int _initCounter = 0;

__forceinline__ __device__ int _IndexAt(int x, int y, int width) { return y * width + x; }
void _SwapPtr(float** l, float** r) { float* temp = *r; *r = *l; *l = temp; }

void _AddSource(int W, int H, float* x, float* s, float dt)
{
    int i, size = (W + 2) * (H + 2);
    for (i = 0; i < size; i++)
        x[i] += dt * s[i];
}

__global__ void SetBoundaryKernel(const int W, const int H, int b, float* x)
{
    for (int i = 1; i <= H; i++)
    {
        x[_IndexAt(0, i, W + 2)]        = b == 1 ? -x[_IndexAt(1, i, W + 2)] : x[_IndexAt(1, i, W + 2)];
        x[_IndexAt(W + 1, i, W + 2)]    = b == 1 ? -x[_IndexAt(W, i, W + 2)] : x[_IndexAt(W, i, W + 2)];
    }
    for (int i = 1; i <= W; i++)
    {
        x[_IndexAt(i, 0, W + 2)]        = b == 2 ? -x[_IndexAt(i, 1, W + 2)] : x[_IndexAt(i, 1, W + 2)];
        x[_IndexAt(i, H + 1, W + 2)]    = b == 2 ? -x[_IndexAt(i, H, W + 2)] : x[_IndexAt(i, H, W + 2)];
    }
    x[_IndexAt(0, 0, W + 2)]            = 0.5 * (x[_IndexAt(1, 0, W + 2)]       + x[_IndexAt(0, 1, W + 2)]);
    x[_IndexAt(0, H + 1, W + 2)]        = 0.5 * (x[_IndexAt(1, H + 1, W + 2)]   + x[_IndexAt(0, H, W + 2)]);
    x[_IndexAt(W + 1, 0, W + 2)]        = 0.5 * (x[_IndexAt(W, 0, W + 2)]       + x[_IndexAt(W + 1, 1, W + 2)]);
    x[_IndexAt(W + 1, H + 1, W + 2)]    = 0.5 * (x[_IndexAt(W, H + 1, W + 2)]   + x[_IndexAt(W + 1, H, W + 2)]);
}

__global__ void DiffuseKernel(float* x, const float* x0, const float a, const int BLOCK_COUNT, const int THREAD_COUNT, const int W, const int H)
{
    int startXIndex = 1 + (W * blockIdx.x) / BLOCK_COUNT;
    int endXIndex = 1 + (W * (blockIdx.x + 1)) / BLOCK_COUNT;
    int startYIndex = 1 + (H * threadIdx.x) / THREAD_COUNT;
    int endYIndex = 1 + (H * (threadIdx.x + 1)) / THREAD_COUNT;
    for (int k = 0; k < 4; k++)
    {
        for (int i = startXIndex; i < endXIndex; i++)
        {
            for (int j = startYIndex; j < endYIndex; j++)
            {
                int index = _IndexAt(i, j, W + 2);
                x[index] = (
                    x0[index] + a * (
                        x[index - 1] +
                        x[index + 1] +
                        x[index - (W + 2)] +
                        x[index + (W + 2)]
                    )
                ) / (1 + 4 * a);
            }
        }
    }
}

void _DiffuseVel(CudaSmokeSim_Context* ctx, float diff, float dt)
{
    float a = dt * diff;

    _SwapPtr(&ctx->dev_u_prev, &ctx->dev_u);
    _SwapPtr(&ctx->dev_v_prev, &ctx->dev_v);
    DiffuseKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_u, ctx->dev_u_prev, a, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    DiffuseKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_v, ctx->dev_v_prev, a, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 1, ctx->dev_u);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 2, ctx->dev_v);
}

void _DiffuseDens(CudaSmokeSim_Context* ctx, float diff, float dt)
{
    float a = dt * diff;

    _SwapPtr(&ctx->dev_dens_prev, &ctx->dev_dens);
    DiffuseKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_dens, ctx->dev_dens_prev, a, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
}

void _DiffuseTemp(CudaSmokeSim_Context* ctx, float diff, float dt)
{
    float a = dt * diff;

    _SwapPtr(&ctx->dev_temp_prev, &ctx->dev_temp);
    DiffuseKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_temp, ctx->dev_temp_prev, a, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
}

__global__ void AdvectKernel(float* d, const float* d0, const float* u, const float* v, const float dt0, const int BLOCK_COUNT, const int THREAD_COUNT, const int W, const int H)
{
    int startXIndex = 1 + (W * blockIdx.x) / BLOCK_COUNT;
    int endXIndex = 1 + (W * (blockIdx.x + 1)) / BLOCK_COUNT;
    int startYIndex = 1 + (H * threadIdx.x) / THREAD_COUNT;
    int endYIndex = 1 + (H * (threadIdx.x + 1)) / THREAD_COUNT;
    for (int i = startXIndex; i < endXIndex; i++)
    {
        for (int j = startYIndex; j < endYIndex; j++)
        {
            float x = i - dt0 * u[_IndexAt(i, j, W + 2)];
            float y = j - dt0 * v[_IndexAt(i, j, W + 2)];
            if (x < 0.5) x = 0.5;
            if (x > W + 0.5) x = W + 0.5;
            int i0 = (int)x;
            int i1 = i0 + 1;
            if (y < 0.5) y = 0.5;
            if (y > H + 0.5) y = H + 0.5;
            int j0 = (int)y;
            int j1 = j0 + 1;

            float s1 = x - i0;
            float s0 = 1 - s1;
            float t1 = y - j0;
            float t0 = 1 - t1;

            d[_IndexAt(i, j, W + 2)] =
                s0 * (t0 * d0[_IndexAt(i0, j0, W + 2)] + t1 * d0[_IndexAt(i0, j1, W + 2)]) +
                s1 * (t0 * d0[_IndexAt(i1, j0, W + 2)] + t1 * d0[_IndexAt(i1, j1, W + 2)]);
        }
    }
}

void _AdvectVel(CudaSmokeSim_Context* ctx, float dt)
{
    float dt0 = dt * ctx->height;
    _SwapPtr(&ctx->dev_u, &ctx->dev_u_prev);
    _SwapPtr(&ctx->dev_v, &ctx->dev_v_prev);
    AdvectKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_u, ctx->dev_u_prev, ctx->dev_u_prev, ctx->dev_v_prev, dt0, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    AdvectKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_v, ctx->dev_v_prev, ctx->dev_u_prev, ctx->dev_v_prev, dt0, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 1, ctx->dev_u);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 2, ctx->dev_v);

    //for (i = 1; i <= W; i++) {
    //    for (j = 1; j <= H; j++) {

    //        oldValSum += d0[_IndexAt(i, j)];
    //        newValSum += d[_IndexAt(i, j)];
    //    }
    //}
    //if (conserve && newValSum != 0.0f)
    //{
    //    float ratio = oldValSum / newValSum;
    //    for (int idx = 0; idx < W * H; idx++)
    //        d[idx] *= ratio;
    //}
}

void _AdvectDens(CudaSmokeSim_Context* ctx, float dt, bool conserve)
{
    float dt0 = dt * ctx->height;
    _SwapPtr(&ctx->dev_dens, &ctx->dev_dens_prev);
    AdvectKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_dens, ctx->dev_dens_prev, ctx->dev_u, ctx->dev_v, dt0, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 1, ctx->dev_dens);
}

void _AdvectTemp(CudaSmokeSim_Context* ctx, float dt, bool conserve)
{
    float dt0 = dt * ctx->height;
    _SwapPtr(&ctx->dev_temp, &ctx->dev_temp_prev);
    AdvectKernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_temp, ctx->dev_temp_prev, ctx->dev_u, ctx->dev_v, dt0, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 1, ctx->dev_temp);
}

__global__ void ProjectP1Kernel(const float* u, const float* v, float* p, float* div, const float h, const int BLOCK_COUNT, const int THREAD_COUNT, const int W, const int H)
{
    int startXIndex = 1 + (W * blockIdx.x) / BLOCK_COUNT;
    int endXIndex = 1 + (W * (blockIdx.x + 1)) / BLOCK_COUNT;
    int startYIndex = 1 + (H * threadIdx.x) / THREAD_COUNT;
    int endYIndex = 1 + (H * (threadIdx.x + 1)) / THREAD_COUNT;
    for (int i = startXIndex; i < endXIndex; i++)
    {
        for (int j = startYIndex; j < endYIndex; j++)
        {
            div[_IndexAt(i, j, W + 2)] = -0.5 * h * (
                u[_IndexAt(i + 1, j, W + 2)] - u[_IndexAt(i - 1, j, W + 2)] +
                v[_IndexAt(i, j + 1, W + 2)] - v[_IndexAt(i, j - 1, W + 2)]
            );
            p[_IndexAt(i, j, W + 2)] = 0;
        }
    }
}

__global__ void ProjectP2Kernel(float* p, const float* div, const int BLOCK_COUNT, const int THREAD_COUNT, const int W, const int H)
{
    int startXIndex = 1 + (W * blockIdx.x) / BLOCK_COUNT;
    int endXIndex = 1 + (W * (blockIdx.x + 1)) / BLOCK_COUNT;
    int startYIndex = 1 + (H * threadIdx.x) / THREAD_COUNT;
    int endYIndex = 1 + (H * (threadIdx.x + 1)) / THREAD_COUNT;
    for (int k = 0; k < 4; k++)
    {
        for (int i = startXIndex; i < endXIndex; i++)
        {
            for (int j = startYIndex; j < endYIndex; j++)
            {
                int index = _IndexAt(i, j, W + 2);
                p[index] = (
                    div[index] +
                    p[index - 1] +
                    p[index + 1] +
                    p[index - (W + 2)] +
                    p[index + (W + 2)]
                ) / 4;
            }
        }
    }
}

__global__ void ProjectP3Kernel(float* u, float* v, const float* p, const float h, const int BLOCK_COUNT, const int THREAD_COUNT, const int W, const int H)
{
    int startXIndex = 1 + (W * blockIdx.x) / BLOCK_COUNT;
    int endXIndex = 1 + (W * (blockIdx.x + 1)) / BLOCK_COUNT;
    int startYIndex = 1 + (H * threadIdx.x) / THREAD_COUNT;
    int endYIndex = 1 + (H * (threadIdx.x + 1)) / THREAD_COUNT;
    for (int i = startXIndex; i < endXIndex; i++)
    {
        for (int j = startYIndex; j < endYIndex; j++)
        {
            u[_IndexAt(i, j, W + 2)] -= 0.5 * (p[_IndexAt(i + 1, j, W + 2)] - p[_IndexAt(i - 1, j, W + 2)]) / h;
            v[_IndexAt(i, j, W + 2)] -= 0.5 * (p[_IndexAt(i, j + 1, W + 2)] - p[_IndexAt(i, j - 1, W + 2)]) / h;
        }
    }
}

void _ProjectVel(CudaSmokeSim_Context* ctx)
{
    float h = 1.0 / ctx->height;

    ProjectP1Kernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_u, ctx->dev_v, ctx->dev_u_prev, ctx->dev_v_prev, h, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 0, ctx->dev_u_prev);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 0, ctx->dev_v_prev);

    ProjectP2Kernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_u_prev, ctx->dev_v_prev, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 0, ctx->dev_u_prev);

    ProjectP3Kernel<<<ctx->blockCount, ctx->threadCount>>>(ctx->dev_u, ctx->dev_v, ctx->dev_u_prev, h, ctx->blockCount, ctx->threadCount, ctx->width, ctx->height);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 1, ctx->dev_u);
    SetBoundaryKernel<<<1, 1>>>(ctx->width, ctx->height, 2, ctx->dev_v);
}

CudaSmokeSim_Context* CudaSmokeSim_Init(int width, int height)
{
    if (_initCounter == 0)
    {
        cudaError_t cudaStatus = cudaSetDevice(0);
        if (cudaStatus != cudaSuccess) {
            fprintf(stderr, "cudaSetDevice failed! Do you have a CUDA-capable GPU installed?");
            return nullptr;
        }
    }
    _initCounter++;

    std::unique_ptr<CudaSmokeSim_Context> ctx = std::make_unique<CudaSmokeSim_Context>();
    ctx->width = width;
    ctx->height = height;
    ctx->totalWidth = width + 2;
    ctx->totalHeight = height + 2;
    ctx->blockCount = 256;
    ctx->threadCount = 128;
    int size = ctx->totalWidth * ctx->totalHeight;

    if (cudaMalloc((void**)&ctx->dev_u,         size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_v,         size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_u_prev,    size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_v_prev,    size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_dens,      size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_dens_prev, size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_temp,      size * sizeof(float)) != cudaSuccess) return nullptr;
    if (cudaMalloc((void**)&ctx->dev_temp_prev, size * sizeof(float)) != cudaSuccess) return nullptr;

    return ctx.release();
}

void CudaSmokeSim_Uninit(CudaSmokeSim_Context* ctx)
{
    if (!ctx)
        return;

    cudaFree(ctx->dev_u);
    cudaFree(ctx->dev_v);
    cudaFree(ctx->dev_u_prev);
    cudaFree(ctx->dev_v_prev);
    cudaFree(ctx->dev_dens);
    cudaFree(ctx->dev_dens_prev);
    cudaFree(ctx->dev_temp);
    cudaFree(ctx->dev_temp_prev);

    _initCounter--;
    if (_initCounter == 0)
        cudaDeviceReset();
}

void CudaSmokeSim_Step(CudaSmokeSim_Context* ctx, CudaSmokeSim_StepData* data)
{
    size_t arrSize = ctx->totalWidth * ctx->totalHeight * sizeof(float);

    cudaError_t cudaStatus;
    cudaStatus = cudaMemcpy(ctx->dev_u,     data->u,    arrSize, cudaMemcpyHostToDevice);
    cudaStatus = cudaMemcpy(ctx->dev_v,     data->v,    arrSize, cudaMemcpyHostToDevice);
    cudaStatus = cudaMemcpy(ctx->dev_dens,  data->dens, arrSize, cudaMemcpyHostToDevice);
    cudaStatus = cudaMemcpy(ctx->dev_temp,  data->temp, arrSize, cudaMemcpyHostToDevice);

    // Velocity step
    if (data->velDiffusion > 0.0f)
        _DiffuseVel(ctx, data->velDiffusion, data->dt);
    _ProjectVel(ctx);
    _AdvectVel(ctx, data->dt);
    _ProjectVel(ctx);

    // Density step
    if (data->densDiffusion > 0.0f)
        _DiffuseDens(ctx, data->densDiffusion, data->dt);
    _AdvectDens(ctx, data->dt, true);

    cudaStatus = cudaDeviceSynchronize();
    cudaStatus = cudaMemcpy(data->u,    ctx->dev_u,     arrSize, cudaMemcpyDeviceToHost);
    cudaStatus = cudaMemcpy(data->v,    ctx->dev_v,     arrSize, cudaMemcpyDeviceToHost);
    cudaStatus = cudaMemcpy(data->dens, ctx->dev_dens,  arrSize, cudaMemcpyDeviceToHost);
    cudaStatus = cudaMemcpy(data->temp, ctx->dev_temp,  arrSize, cudaMemcpyDeviceToHost);
}