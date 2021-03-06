/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#include "../error_handling.h"
#include "../cuda_utils.h"

namespace FT{
    namespace Pop{
        namespace Op{
                           		
            __global__ void Not( bool * x, size_t idx, size_t N)
            {                    
                for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < N; i += blockDim.x * gridDim.x)
                {
                    x[(idx-1)*N+i] = ! x[(idx-1)*N+i] ;
                }
                return;
            }
            void GPU_Not( bool * x, size_t idx, size_t N)
            {
                Not<<< DIM_GRID, DIM_BLOCK >>>(x, idx, N);
            }
            /// Evaluates the node and updates the stack states. 
            /* void NodeNot::evaluate(const MatrixXf& X, const VectorXf& y, vector<ArrayXf>& stack_f, */ 
            /*         vector<ArrayXb>& stack_b) */
            /* { */
            /*     ArrayXb x1 = stack_b.back(); stack_b.pop_back(); */
            /*     // evaluate on the GPU */
            /*     ArrayXb result = ArrayXb(x1.size()); */
            /*     size_t N = result.size(); */
            /*     bool * dev_res; */
            /*     int numSMs; */
            /*     cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, 0); */
            /*     // allocate device arrays */
            /*     bool * dev_x1 ; */ 
            /*     HANDLE_ERROR(cudaMalloc((void **)& dev_x1, sizeof(bool)*N)); */
            /*     HANDLE_ERROR(cudaMalloc((void **)&dev_res, sizeof(bool)*N)); */
            /*     // Copy to device */
            /*     HANDLE_ERROR(cudaMemcpy(dev_x1, x1.data(), sizeof(bool)*N, cudaMemcpyHostToDevice)); */

            /*     Not<<< 32*numSMs, 128 >>>(dev_x1, dev_res, N); */
               
            /*     // Copy to host */
            /*     HANDLE_ERROR(cudaMemcpy(result.data(), dev_res, sizeof(bool)*N, cudaMemcpyDeviceToHost)); */
                
            /*     stack_b.push_back(result); */
            /*     // Free memory */
            /*     cudaFree(dev_x1); cudaFree(dev_res); */
            /* } */

        }	
    }
}

