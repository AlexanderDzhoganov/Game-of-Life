#include "../include/headers.h"

void gameOfLife(gof_data gof) {
	int width = gof.width;
	int height = gof.height;

	for(int x=1; x<width-1; x++) {
		for(int y=1; y<height-1; y++) {
			uint8_t p = gof.inputBuffer[x+y*width];
			uint8_t neighbors = 0;
			neighbors += gof.inputBuffer[(x+1)+y*width];
			neighbors += gof.inputBuffer[(x-1)+y*width];
			neighbors += gof.inputBuffer[x+(y+1)*width];
			neighbors += gof.inputBuffer[x+(y-1)*width];
			neighbors += gof.inputBuffer[(x+1)+(y+1)*width];
			neighbors += gof.inputBuffer[(x-1)+(y-1)*width];
			neighbors += gof.inputBuffer[(x-1)+(y+1)*width];
			neighbors += gof.inputBuffer[(x+1)+(y-1)*width];

			uint8_t final = 0;
			
			if(p != 0) {
				for(int i=0; i<gof.S_n; i++) {
					if(neighbors == gof.S[i]) {
						final = 1;
					}
				}
			} else {
				for(int i=0; i<gof.B_n; i++) {
					if(neighbors == gof.B[i]) {
						final = 1;
						break;
					}
				}
			}
	
			gof.outputBuffer[x+y*width] = final;
		}
	}
}

CUDA_CPUEmu::CUDA_CPUEmu(int width_, int height_) {
	width = width_;
	height = height_;
	bufferSize = sizeof(uint8_t)*width*height;
	
	cudaOutputBuffer = new uint8_t[width*height];
	memset(cudaOutputBuffer, 0, bufferSize);
	cudaInputBuffer = new uint8_t[width*height];
}

CUDA_CPUEmu::~CUDA_CPUEmu() {

}

void CUDA_CPUEmu::getInput(uint8_t* buffer) {
	cudaInputBuffer = buffer;
}

void CUDA_CPUEmu::runGameOfLife(uint8_t* returnBuffer, int* B, int B_n, int* S, int S_n) {
	gof_data gof;
	gof.inputBuffer = cudaInputBuffer;
	gof.outputBuffer = cudaOutputBuffer;
	gof.height = height;
	gof.width = width;

	memcpy(gof.B, B, sizeof(int)*B_n);
	memcpy(gof.S, S, sizeof(int)*S_n);
	gof.B_n = B_n;
	gof.S_n = S_n;

	gameOfLife(gof);
	memcpy(returnBuffer, cudaOutputBuffer, bufferSize);
}