// Template, major revision 3
// IGAD/NHTV - Jacco Bikker - 2006-2009
#include "glew.h" 
#include "gl.h"
#include "../include/headers.h"


using namespace Tmpl8;

CMap cmap;

void Game::Init()
{
	scene = new GameNode();
	grid = new Grid(this, floor(double(SCRWIDTH/16))*16, floor(double(SCRHEIGHT/16))*16);
	debug = new DebugConsole(this, 16, 10, 10, 750);
	ui = new UIMgr(this);

	UIWindow* w5 = ui->createWindow(vec2(879.f, 10.f), vec2(135.f, 200.f), "GUI", false);
	UIMenu* guimenu = ui->createMenu(w5, 25.f, "show_gui");
	guimenu->addItem("SHOW");
	guimenu->addItem("HIDE");

	UIMenu* colormenu = ui->createMenu(w5, 85.f, "colors_menu");
	colormenu->addItem("RED");
	colormenu->addItem("GREEN");
	colormenu->addItem("BLUE");
	colormenu->addItem("0xE7B53B");
	colormenu->addItem("0xC5AF91");
	CInt colors_menu("colors_menu");
	*colors_menu = 3;

	UIWindow* w = ui->createWindow(vec2(10.f, 10.f), vec2(175.f, 730.f), "RULES");
	UIMenu* rules = ui->createMenu(w, 25.f, "rules_menu");
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

	UIWindow* w3 = ui->createWindow(vec2(879.f, 90.f), vec2(135.f, 130.f), "ACTIONS");
	UIMenu* stuff = ui->createMenu(w3, 25.f, "actions_menu");
	stuff->addItem("CLEAR");
	stuff->addItem("SEED 1X");
	stuff->addItem("SEED 10X");
	stuff->addItem("SEED 100X");
	stuff->addItem("SEED 1000X");
	CInt actions_menu("actions_menu");
	*actions_menu = -1;

	UIWindow* w4 = ui->createWindow(vec2(879.f, 225.f), vec2(135.f, 130.f), "FPS LOCK");
	UIMenu* fpsmenu = ui->createMenu(w4, 25.f, "fps_menu");
	fpsmenu->addItem("UNLOCKED");
	fpsmenu->addItem("PAUSE");
	fpsmenu->addItem("1 FPS");
	fpsmenu->addItem("12 FPS");
	fpsmenu->addItem("50 FPS");

	UIWindow* w6 = ui->createWindow(vec2(879.f, 360.f), vec2(135.f, 110.f), "BRUSH SIZE");
	UIMenu* brushmenu = ui->createMenu(w6, 25.f, "brush_menu");
	brushmenu->addItem("SMALL");
	brushmenu->addItem("MEDIUM");
	brushmenu->addItem("LARGE");
	brushmenu->addItem("HUGE");
	CInt brush_menu("brush_menu");
	*brush_menu = 1;

	UIWindow* w2 = ui->createWindow(vec2(879.f, 360.f), vec2(135.f, 75.f), "COMPUTE ON");
	UIMenu* cpugpu = ui->createMenu(w2, 25.f, "use_cuda");
	cpugpu->addItem("CPU");
	cpugpu->addItem("GPU");

	UIWindow* w7 = ui->createWindow(vec2(879.f, 360.f), vec2(135.f, 50.f), "QUIT");
	UIMenu* quitmenu = ui->createMenu(w7, 25.f, "quit_menu");
	quitmenu->addItem("QUIT");
	CInt quit_menu("quit_menu");
	*quit_menu = 2;

	CInt cuda_dev_n("cuda_dev_n");
	CInt use_cuda("use_cuda");
	if(*cuda_dev_n > 0 && *cuda_dev_n < 10) {
		*use_cuda = 1;
	} else {
		*use_cuda = 0;
	}

	frameCounter = 0;
	frameTimer = 0.f;
	shiftDown = false;
	*time_scale = 1.f;
	timeAccumulator = 0.f;
	logo = new Surface("cuda.png");
	mouseMask = 0;
}

void Game::Tick( float a_DT )
{
	frameCounter++;
	frameTimer += a_DT;
	if(frameTimer > 1000.f) {
		*fps = frameCounter;
		frameCounter = 0;
		frameTimer = 0.f;
	}
	a_DT *= *time_scale;
	a_DT += 0.1f;

	m_Screen->Clear(0);
	scene->update(a_DT);

	CInt use_cuda("use_cuda");
	if(*use_cuda)
		logo->CopyToColorKey(m_Screen, SCRWIDTH-(logo->GetWidth()), SCRHEIGHT-(logo->GetHeight()));

	debug->update(a_DT);

	CInt quit_menu("quit_menu");
	if(*quit_menu == 0) exit(0);

}

void Game::KeyDown(unsigned int unicode, unsigned int scancode) {
		if(scancode == 225) shiftDown = true;
		else if(!shiftDown) {
			debug->getKey(unicode);
		} else if(shiftDown) {
			if(unicode == '4') debug->getKey('$');
			if(unicode == '3') debug->getKey('#');
			if(unicode == '-') debug->getKey('_');
		}
}

void Game::KeyUp(unsigned int unicode, unsigned int scancode) {
	if(scancode == 225) shiftDown = false;
}

void Game::MouseMove( unsigned int x, unsigned int y ) {
	mousePos[0] = x;
	mousePos[1] = y;
}

void Game::MouseUp( unsigned int button ) {
	mouseMask = 0;
}

void Game::MouseDown( unsigned int button ) {
	mouseMask = 1;
}

GameNode* Game::addNode(GameNode* node, GameNode* parent) {
	if(parent) {
		parent->addChild(node);
	} else {
		scene->addChild(node);
	}
	return node;
}

GameNode* Game::getScene() {
	return scene;
}

Surface* Game::getScreen() {
	return m_Screen;
}

DebugConsole* Game::getDebug() {
	return debug;
}

UIMgr* Game::getUI() {
	return ui;
}

vec2 Game::getMousePos() {
	return mousePos;
}

int Game::getMouseMask() {
	return mouseMask;
}