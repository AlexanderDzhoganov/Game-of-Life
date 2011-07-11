#include "../include/headers.h"

typedef unsigned char uint8_t;

texture<uint8_t, 2, cudaReadModeElementType> inputTex;

__global__ void gameOfLife(gof_data* gof) {
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	uint8_t p = tex2D(inputTex, x, y);
	uint8_t neighbors = 0;
	neighbors += tex2D(inputTex, x+1, y);
	neighbors += tex2D(inputTex, x-1, y);
	neighbors += tex2D(inputTex, x, y+1);
	neighbors += tex2D(inputTex, x, y-1);
	neighbors += tex2D(inputTex, x+1, y+1);
	neighbors += tex2D(inputTex, x-1, y-1);
	neighbors += tex2D(inputTex, x-1, y+1);
	neighbors += tex2D(inputTex, x+1, y-1);

	__syncthreads();

	uint8_t final = 0;
			
	if(p != 0) {
		for(int i=0; i<gof->S_n; i++) {
			if(neighbors == gof->S[i]) {
				final = 1;
			}
		}
	} else {
		for(int i=0; i<gof->B_n; i++) {
			if(neighbors == gof->B[i]) {
				final = 1;
				break;
			}
		}
	}

	__syncthreads();
	gof->outputBuffer[x+y*gof->width] = final;
}

CUDA::CUDA(int width_, int height_) {
	width = width_;
	height = height_;
	bufferSize = sizeof(uint8_t)*width*height;
	cudaMalloc((void**)&cudaOutputBuffer, bufferSize);
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(8, 0, 0, 0, cudaChannelFormatKindUnsigned);
	cudaMallocArray(&cudaInputArray, &channelDesc, width, height);
	inputTex.addressMode[0] = cudaAddressModeWrap;
    inputTex.addressMode[1] = cudaAddressModeWrap;
    inputTex.filterMode = cudaFilterModePoint;
    inputTex.normalized = false;
	cudaBindTextureToArray(inputTex, cudaInputArray, channelDesc);
}

CUDA::~CUDA() {

}

void CUDA::getInput(uint8_t* buffer) {
	cudaMemcpyToArrayAsync(cudaInputArray, 0, 0, buffer, bufferSize, cudaMemcpyHostToDevice); 
}

void CUDA::runGameOfLife(uint8_t* returnBuffer, int* B, int B_n, int* S, int S_n) {
	dim3 grid(width/16, height/16);
	dim3 block(16, 16);

	gof_data* gof;
	cudaMalloc((void**)&gof, sizeof(gof_data));

	gof_data l_gof;
	l_gof.inputBuffer = cudaInputBuffer;
	l_gof.outputBuffer = cudaOutputBuffer;
	l_gof.height = height;
	l_gof.width = width;

	memcpy(l_gof.B, B, sizeof(int)*B_n);
	memcpy(l_gof.S, S, sizeof(int)*S_n);
	l_gof.B_n = B_n;
	l_gof.S_n = S_n;

	cudaMemcpy(gof, &l_gof, sizeof(gof_data), cudaMemcpyHostToDevice);
	gameOfLife <<< grid, block >>> (gof);
	cudaMemcpy(returnBuffer, cudaOutputBuffer, bufferSize, cudaMemcpyDeviceToHost);
}