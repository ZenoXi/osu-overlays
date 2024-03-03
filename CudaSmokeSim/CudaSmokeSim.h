#ifndef CUDA_SMOKE_SIM_H
#define CUDA_SMOKE_SIM_H

#ifdef __cplusplus
extern "C" {
#endif

    struct CudaSmokeSim_Context
    {
        int width;
        int height;
        int totalWidth;
        int totalHeight;
        int blockCount;
        int threadCount;

        float* dev_u;
        float* dev_v;
        float* dev_u_prev;
        float* dev_v_prev;
        float* dev_dens;
        float* dev_dens_prev;
        float* dev_temp;
        float* dev_temp_prev;
    };

    struct CudaSmokeSim_StepData
    {
        float* u;
        float* v;
        float* dens;
        float* temp;

        float dt;
        float velDiffusion;
        float densDiffusion;
        float tempDiffusion;
    };

    __declspec(dllexport) CudaSmokeSim_Context* CudaSmokeSim_Init(int width, int height);
    __declspec(dllexport) void CudaSmokeSim_Step(CudaSmokeSim_Context* ctx, CudaSmokeSim_StepData* data);
    __declspec(dllexport) void CudaSmokeSim_Uninit(CudaSmokeSim_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif // CUDA_SIM_H