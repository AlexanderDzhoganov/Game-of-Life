#include "../include/headers.h"

typedef unsigned char uint8_t;

void decode_rule(char* rule, int* B, int* B_n, int* S, int* S_n) {
	int len = strlen(rule);

	bool fillB = false;
	bool doneB = false;
	bool fillS = false;

	*B_n = 0;
	*S_n = 0;
	
	char buffer[2];
	buffer[1] = 0;

	for(int x=0; x<len; x++) {
		if(!fillB) if(rule[x] == 'B') {
			fillB = true;
			continue;
		}
		if(fillB) {
			if(rule[x] == '/') {
				fillB = false;
				fillS = true;
				doneB = true;
				x++;
				continue;
			}
			buffer[0] = rule[x];
			B[*B_n] = atoi(buffer);
			(*B_n)++;
			continue;
		}
		if(fillS) {
			buffer[0] = rule[x];
			S[*S_n] = atoi(buffer);
			(*S_n)++;
			continue;
		}	
	}
}



//extern void CUDA_gameOfLife(float* currentBuffer, float* nextBuffer, size_t pitch);
Grid::Grid(Tmpl8::Game* game_n, int width_n, int height_n) : \
	zoom("zoom"),\
	draw_type("draw_type"),\
	use_cuda("use_cuda"),\
	memcpy_to_scr("memcpy_to_scr"),\
	rules_menu("rules_menu"),\
	actions_menu("actions_menu"),\
	fps_menu("fps_menu"),\
	brush_menu("brush_menu"),\
	colors_menu("colors_menu")\
{
	game = game_n;
	game->addNode(this);
	width = width_n;
	height = height_n;
	bufferSize = sizeof(uint8_t)*width*height;
	initialBuffer = new uint8_t[width*height];
	drawBuffer = new uint8_t[width*height];
	drawSurface = new Tmpl8::Surface(width, height);

	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++) {
			if(rand()%17 == 7) {
				initialBuffer[x+y*width] = 1;
			} else initialBuffer[x+y*width] = 0;

			drawBuffer[x+y*width] = 0;
		}
	}
	CInt cuda_dev_n("cuda_dev_n", true, true);
	int tmp;
	cudaGetDeviceCount(&tmp);
	*cuda_dev_n = tmp;

	cuda = new CUDA(width, height);
	cudaemu = new CUDA_CPUEmu(width, height);
	
	timeAcc = 0.f;
	*zoom = 1.f;
	*draw_type = 0;

	*memcpy_to_scr = 0;
	
	rules = new Rules();
	rules->addItem("B3/S23");
	rules->addItem("B3/S12345");
	rules->addItem("B257/S27");
	rules->addItem("B018/S018");
	rules->addItem("B0578/S045");
	rules->addItem("B0578/S123456");
	rules->addItem("B1/S0123456");
	rules->addItem("B1/S1");
	rules->addItem("B123567/S0238");
	rules->addItem("B13456/S01356");
	rules->addItem("B135/S135");
	rules->addItem("B1357/S1357");
	rules->addItem("B137/S45678");
	rules->addItem("B2/S");
	rules->addItem("B234/S");
	rules->addItem("B236/S0468");
	rules->addItem("B257/S27");
	rules->addItem("B3/S012345678");
	rules->addItem("B3/S234");
	rules->addItem("B3/S45678");
	rules->addItem("B34/S03456");
	rules->addItem("B34/S34");
	rules->addItem("B345/S4567");
	rules->addItem("B345/S5");
	rules->addItem("B35678/S5678");
	rules->addItem("B357/S1358");
	rules->addItem("B357/S238");
	rules->addItem("B36/S125");
	rules->addItem("B36/S23");
	rules->addItem("B367/S2346");
	rules->addItem("B3678/S235678");
	rules->addItem("B3678/S235678");
	rules->addItem("B3678/S34678");
	rules->addItem("B368/S245");
	rules->addItem("B45678/S2345");

	timeTillNextFrame = 0.f;
	fps_lock = 0;
	rules_menu_prev = -1337;
}

Grid::~Grid() {
	delete [] initialBuffer;
	delete [] drawBuffer;
	delete cuda;
	delete cudaemu;
}

void Grid::update(float frameDelta) {
	bool doFrame = true;
	
	switch(*fps_menu) {
	case 0:
		fps_lock = 0;
		break;
	case 1:
		fps_lock = -1;
		break;
	case 2:
		fps_lock = 1;
		break;
	case 3:
		fps_lock = 12;
		break;
	case 4:
		fps_lock = 50;
		break;
	default:
		fps_lock = 50;
		break;
	}

	if(fps_lock > 0) {
		timeTillNextFrame -= frameDelta/1000.f;
		doFrame = false;
		if(timeTillNextFrame < 0.f) {
			timeTillNextFrame = 1.f/float(fps_lock);
			doFrame = true;
		} 
	} else if(fps_lock == -1) {
		doFrame = false;
	}

	vec2 mousePos = game->getMousePos();
	int mouseMask = game->getMouseMask();

	if(mouseMask && game->getUI()->isMouseFree()) {
		switch(*brush_menu) {
		case 0:
			brush_size = 4;
			break;
		case 1:
			brush_size = 5;
			break;
		case 2:
			brush_size = 10;
			break;
		case 3:
			brush_size = 25;
			break;
		default:
			brush_size = 4;
			break;
		}

		int m_x = mousePos[0];
		int m_y = mousePos[1];
		int halfBrush = floor(float(brush_size)*0.5f);
		for(int x=-halfBrush; x<halfBrush; x++) {
			for(int y=-halfBrush; y<halfBrush; y++) {
				int idx = (m_x+x)+(m_y+y)*width;
				if(idx > 0 && idx < width*height)
					initialBuffer[idx] = 1;
			}
		}
	}

	if(*actions_menu != -1) {
		switch(*actions_menu) {
		case 0:
			clearAndSeed(0);
			break;
		case 1:
			clearAndSeed(250);
			break;
		case 2:
			clearAndSeed(2500);
			break;
		case 3:
			clearAndSeed(25000);
			break;
		case 4:
			clearAndSeed(250000);
			break;
		default:
			clearAndSeed(2500);
			break;
		}
		*actions_menu = -1;
	}

	if(*rules_menu != rules_menu_prev) {
		decode_rule(rules->fetchItem(*rules_menu), B, &B_n, S, &S_n);
	}

	rules_menu_prev = *rules_menu;

	if(doFrame) {
		CInt cuda_dev_n("cuda_dev_n");
		if(*use_cuda && *cuda_dev_n > 0 && *cuda_dev_n < 10) {
			cuda->getInput(initialBuffer);
			cuda->runGameOfLife(initialBuffer, B, B_n, S, S_n);
		} else {
			*use_cuda = false;
			cudaemu->getInput(initialBuffer);
			cudaemu->runGameOfLife(initialBuffer, B, B_n, S, S_n);
		}
	}

	drawSelf(vec2(0, 0));
}

void Grid::drawSelf(vec2 origin) {
	unsigned int* buf = (unsigned int*)drawSurface->GetBuffer();

	unsigned long color;
	switch(*colors_menu) {
	case 0:
		color = 0xFFFF0000;
		break;
	case 1:
		color = 0xFF00FF00;
		break;
	case 2:
		color = 0xFF0000FF;
		break;
	case 3:
		color = 0xFFE7B53B;
		break;
	case 4:
		color = 0xFFC5AF91;
		break;
	default:
		color = 0xFFFF0000;
		break;
	}

	for(int x=0; x<width*height; x++) {
			if(initialBuffer[x]) {
				buf[x] = color;
			} else {
				buf[x] = 0xFF000000;
			}
	}

	
	if(*memcpy_to_scr == 0) drawSurface->CopyTo(game->getScreen(), origin[0], origin[1]);
	else memcpy(game->getScreen()->GetBuffer(), drawSurface->GetBuffer(), sizeof(unsigned int)*width*height);
}

void Grid::clearAndSeed(int seedCount) {
	for(int x=0; x<width*height; x++) {	
		initialBuffer[x] = 0;
	}
	
	for(int i=0; i<seedCount; i++) {
		int x = rand()%width;
		int y = rand()%height;
		initialBuffer[x+y*width] = 1;
	}
}