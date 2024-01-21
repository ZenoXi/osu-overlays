#pragma once

#include "Navigation.h"

#include "Window/WindowsEx.h"

#include <string>
#include <algorithm>

//#define PI 3.14159265f

// Helper structs
struct LineFunc
{
    float k;
    float b;
    bool vertical = false;

    float ValueY(float x) const { return k * x + b; }
    float ValueX(float y) const { return (y - b) / k; }
    void VecToK(Pos2D<float> vec) {
        if (vec.x == 0.0f) {
            k = 0.0f;
            vertical = true;
        }
        else {
            k = vec.y / vec.x;
        }
    }
    void CalcB(float x, float y) {
        if (vertical) {
            b = x;
        }
        else if (!vertical && k == 0.0f) {
            b = y;
        }
        else {
            b = y - k * x;
        }
    }
    LineFunc GetPerpendicular()
    {
        LineFunc per;
        if (vertical) {
            per.vertical = false;
            per.k = 0.0f;
        }
        else if (!vertical && k == 0.0f) {
            per.vertical = true;
            per.k = 0.0f;
        }
        else {
            per.vertical = false;
            per.k = -1.0f / k;
        }
    }
};

// Number manipulation
int absv(int v);
template <typename T>
int sign(T val);
bool is_equal_f(float f1, float f2, float epsilon = 0.000001f);
bool is_equal_d(double d1, double d2, double epsilon = 0.000000001f);
float sin_norm(float f);
float cos_norm(float f);
float cos_norm_scaled(float f, float scale);
float safe_sqrt(float f);
template <typename T>
T int_pow(T num, int power);

// Algebra
template<typename T>
std::vector<double> gauss_elim(std::vector<std::vector<T>> data, std::vector<T> b);

// Vectors
template<typename T>
T dot_product(Pos2D<T> v1, Pos2D<T> v2);
template<typename T>
T cross_product(Pos2D<T> v1, Pos2D<T> v2);
std::pair<float, float> point_as_vectors(Pos2D<float> v1, Pos2D<float> v2, Pos2D<float> point);

// Geometry
template<typename T>
int distance(Pos2D<T> pos1, Pos2D<T> pos2);
template<typename T>
float distance_real(Pos2D<T> pos1, Pos2D<T> pos2);
int distance_manh(Pos2D<int> pos1, Pos2D<int> pos2);
template<typename T>
float triangle_height(const Pos2D<T>& A, const Pos2D<T>& B, const Pos2D<T>& C);

// Complex geometry
float direction(Pos2D<float> startPos, Pos2D<float> endPos);
Pos2D<float> point_in_direction(Pos2D<float> originPos, float direction, float distance);
Pos2D<float> point_rotated_by(Pos2D<float> rotationAxis, Pos2D<float> p, float rotation);
template<typename T>
bool ray_to_right_intersects_line_segment(const Pos2D<T>& p, const Pos2D<T>& l1, const Pos2D<T>& l2);
template<typename T>
bool point_is_on_line_segment(const Pos2D<T>& p, const Pos2D<T>& l1, const Pos2D<T>& l2);
template<typename T>
Pos2D<T> line_segment_intersection(const Pos2D<T>& s1, const Pos2D<T>& e1, const Pos2D<T>& s2, const Pos2D<T>& e2, float epsilon = 0.000001f);

// Bounds
bool add_bounds(Pos2D<int> &pos, const RECT &bounds);
bool add_bounds(int &value, const int &min_v, const int &max_v);
bool add_bounds(float &value, const float &min_v, const float &max_v);
bool check_bounds(const Pos2D<int> &pos, const RECT &bounds);
bool check_bounds(const int &value, const int &min_v, const int &max_v);
bool check_bounds(const float &value, const float &min_v, const float &max_v);


// TEMPLATED FUNCTION DEFINITIONS //

// Number manipulation
template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

template <typename T>
T int_pow(T num, int power)
{
    T result = 1;
    for (int i = 0; i < power; i++)
        result *= num;
    return result;
}

// Algebra
// For square matrices
template<typename T>
std::vector<double> gauss_elim(std::vector<std::vector<T>> A, std::vector<T> b)
{
    unsigned N = A.size();

    // Append b vector to A matrix
    for (unsigned i = 0; i < N; i++) {
        A[i].push_back(b[i]);
    }

    // Find average of all matrix elements
    double average = 0.0f;
    for (unsigned i = 0; i < N; i++) {
        for (unsigned j = 0; j < N + 1; j++) {
            average += A[i][j];
        }
    }
    average /= (N * (N + 1));
    average = abs(average);

    // Divide every element by the average
    if (false && average != 0.0f) {
        for (unsigned i = 0; i < N; i++) {
            for (unsigned j = 0; j < N + 1; j++) {
                A[i][j] /= average;
            }
        }
    }

    // Stage 1 ()
    //for (unsigned i = 0; i < n - 1; i++) {
    //    for (unsigned j = i + 1; j < n; j++) {
    //        float ratio = -A[j][i] / A[i][i];
    //        A[j][i] = 0.0f;
    //        for (unsigned k = i + 1; k < n; k++) {
    //            A[j][k] += A[i][k] * ratio;
    //        }
    //    }
    //}

    // Stage 2
    for (int k = 0; k < N; k++)
    {
        // Initialize maximum value and index for pivot 
        int i_max = k;
        double v_max = A[i_max][k];

        /* find greater amplitude for pivot if any */
        for (int i = k + 1; i < N; i++)
            if (abs(A[i][k]) > v_max)
                v_max = A[i][k], i_max = i;

        /* if a prinicipal diagonal element  is zero,
         * it denotes that matrix is singular, and
         * will lead to a division-by-zero later. */
        if (A[k][i_max] == 0.0)
            continue;
        //    return k; // Matrix is singular 

        /* Swap the greatest value row with current row */
        if (i_max != k)
            std::swap(A[k], A[i_max]);

        for (int i = k + 1; i < N; i++)
        {
            /* factor f to set current row kth element to 0,
             * and subsequently remaining kth column to 0 */
            double f = A[i][k] / A[k][k];

            /* subtract fth multiple of corresponding kth
               row element*/
            for (int j = k + 1; j <= N; j++) {
                A[i][j] -= A[k][j] * f;
            }

            /* filling lower triangular matrix with zeros*/
            A[i][k] = 0.0f;
        }
        //print(mat);        //for matrix state 
    }

    std::vector<double> x;
    x.resize(N, 0.0f);

    for (int i = N - 1; i >= 0; i--)
    {
        /* start with the RHS of the equation */
        x[i] = A[i][N];

        /* Initialize j to i+1 since matrix is upper
           triangular*/
        for (int j = i + 1; j < N; j++)
        {
            /* subtract all the lhs values
             * except the coefficient of the variable
             * whose value is being calculated */
            x[i] -= A[i][j] * x[j];
        }

        /* divide the RHS by the coefficient of the
           unknown being calculated */
        if (A[i][i] == 0.0)
        {
            int M = 5;
            M++;
        }

        x[i] = x[i] / A[i][i];
    }

    return x;
}

// Vectors
template<typename T>
T dot_product(Pos2D<T> v1, Pos2D<T> v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}
template<typename T>
T cross_product(Pos2D<T> v1, Pos2D<T> v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

// Geometry
template<typename T>
float distance_real(Pos2D<T> pos1, Pos2D<T> pos2)
{
    return sqrt(((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y)));
}

template<typename T>
int distance(Pos2D<T> pos1, Pos2D<T> pos2)
{
    return trunc(sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y)));
}

// Complex geometry
template<typename T>
bool ray_to_right_intersects_line_segment(const Pos2D<T>& p, const Pos2D<T>& l1, const Pos2D<T>& l2)
{
    if (l1.x < p.x && l2.x < p.x) return false;
    if (l1.y > p.y && l2.y > p.y) return false;
    if (l1.y < p.y && l2.y < p.y) return false;
    if (l1.x >= p.x && l2.x >= p.x) return true;
    if ((l1.x < p.x && l2.x > p.x) || (l1.x > p.x && l2.x < p.x)) {
        float k1; // Slope of l1 -> l2 or l2 -> l1
        float k2; // Slope of l1 -> p or l2 -> p
        Pos2D<T> pl; // Point to the left of p
        Pos2D<T> pr; // Point to the right of p

        if (l1.x < l2.x) {
            pl = l1;
            pr = l2;
        }
        else if (l2.x < l1.x) {
            pl = l2;
            pr = l1;
        }
        else {
            return true;
        }

        k1 = (pr.y - pl.y) / (pr.x - pl.x);
        k2 = (p.y - pl.y) / (p.x - pl.x);

        if (fabs(k1) > fabs(k2)) return false;
        else return true;
    }

    // Should never be hit
    return true;
}

template<typename T>
bool point_is_on_line_segment(const Pos2D<T>& p, const Pos2D<T>& l1, const Pos2D<T>& l2)
{
    if (l1.x == l2.x) {
        if (p.x != l1.x) {
            return false;
        }
        else {
            Pos2D<T> pu;
            Pos2D<T> pd;
            if (l1.y > l2.y) {
                pu = l1;
                pd = l2;
            }
            else {
                pu = l2;
                pd = l1;
            }

            if (p.y < pu.y) return false;
            if (p.y > pd.y) return false;
            return true;
        }
    }

    float k1 = (l2.y - l1.y) / (l2.x - l1.x);
    float k2 = (p.y - l1.y) / (p.x - l1.x);

    if (k1 != k2) return false;

    // Any point out of these bounds cannot be on line segment
    Pos2D<T> boundUpperLeft = { min(l1.x, l2.x), min(l1.y, l2.y) };
    Pos2D<T> boundBottomRight = { max(l1.x, l2.x), max(l1.y, l2.y) };

    if (p.x < boundUpperLeft.x || p.x > boundBottomRight.x) return false;
    if (p.y < boundUpperLeft.y || p.y > boundBottomRight.y) return false;

    return true;
}

template<typename T>
Pos2D<T> line_segment_intersection(const Pos2D<T>& s1, const Pos2D<T>& e1, const Pos2D<T>& s2, const Pos2D<T>& e2, float epsilon)
{
    if (e1.x == s1.x && e2.x == s2.x) return { 0, 0, true };

    Pos2D<T> v1 = e1 - s1;
    Pos2D<T> v2 = e2 - s2;

    // Any intersection out of these bounds is invalid
    Pos2D<T> boundUpperLeft;
    Pos2D<T> boundBottomRight;
    {
        using namespace std;
        // Set bounds with a little leeway because floats are cunts
        boundUpperLeft = { max(min(s1.x, e1.x), min(s2.x, e2.x)) - epsilon, max(min(s1.y, e1.y), min(s2.y, e2.y)) - epsilon, false };
        boundBottomRight = { min(max(s1.x, e1.x), max(s2.x, e2.x)) + epsilon, min(max(s1.y, e1.y), max(s2.y, e2.y)) + epsilon, false };
    }

    // Return position as invalid if bounds are invalid
    if (boundBottomRight.x < boundUpperLeft.x) return { 0, 0, true };
    if (boundBottomRight.y < boundUpperLeft.y) return { 0, 0, true };

    float x;
    float y;

    // k == inf cases
    if (e1.x == s1.x && e2.x != s2.x) {
        float k = (e2.y - s2.y) / (e2.x - s2.x);
        float c = s2.y - (s2.x * k);
        x = e1.x;
        y = k * x + c;
    }
    else if (e2.x == s2.x && e1.x != s1.x) {
        float k = (e1.y - s1.y) / (e1.x - s1.x);
        float c = s1.y - (s1.x * k);
        x = e2.x;
        y = k * x + c;
    }
    // Normal case
    else {
        // Calculate functions
        float k1 = (e1.y - s1.y) / (e1.x - s1.x);
        float k2 = (e2.y - s2.y) / (e2.x - s2.x);
        if (k1 == k2) return { 0, 0, true };

        float c1 = s1.y - (s1.x * k1);
        float c2 = s2.y - (s2.x * k2);

        // Solve for line intersection
        x = (c2 - c1) / (k1 - k2);
        y = k1 * x + c1; // k2 * x + c2 also works
    }

    // Check bounds
    if (x < boundUpperLeft.x || y < boundUpperLeft.y) return { 0, 0, true };
    if (x > boundBottomRight.x || y > boundBottomRight.y) return { 0, 0, true };

    return Pos2D<T>(x, y, false);
}

// Also works as distance from point to line where C is the point and AB is the line
template<typename T>
float triangle_height(const Pos2D<T>& A, const Pos2D<T>& B, const Pos2D<T>& C)
{
    float a = (B - C).vector_length();
    float b = (A - C).vector_length();
    float c = (A - B).vector_length();
    float p = (a + b + c) / 2.0f;

    if (p < a) p = a;
    if (p < b) p = b;
    if (p < c) p = c;

    return 2.0f * sqrt(fabs(p)) * sqrt(fabs(p - a)) * sqrt(fabs(p - b)) * sqrt(fabs(p - c)) / c;
}