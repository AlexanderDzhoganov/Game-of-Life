#include "../include/headers.h"

// UIWidget

void UIWidget::update(float frameDelta) {
	GameNode::update(frameDelta);
}

// UIWindow

UIWindow::UIWindow(Tmpl8::Game* game_n, vec2 init_pos, vec2 size_n, char* title_n, bool hidable_n) : show_gui("show_gui") {
	game = game_n;
	pos = init_pos;
	size = size_n;
	title = title_n;
	bg = new Tmpl8::Surface(size[0], size[1]);
	bg->Clear(BG_COLOR);
	title_bg = new Tmpl8::Surface(size[0], WINDOW_TITLE_SIZE_Y);
	title_bg->Clear(WINDOW_TITLE_COLOR);
	Tmpl8::Surface* tmp = new Tmpl8::Surface(512, 20);
	tmp->InitCharset();
	tmp->Clear(0);
	tmp->Print(title, 0, 0, 0xffffff);
	title_surf = new Tmpl8::Sprite(tmp, 1);
	active = false;
	move = false;
	hidable = hidable_n;

	pulledUp = true;
	pullDownAnim = false;
	pullUpAnim = false;
	pullDownPos = size[1];
	keyDelay = 0.f;
	hover = false;
}

UIWindow::~UIWindow() {

}

void UIWindow::update(float frameDelta) {
	if(*show_gui && hidable) return;

	vec2 mousePos = game->getMousePos();
	int mouseMask = game->getMouseMask();
	
	if(keyDelay) {
		keyDelay -= frameDelta;
		if(keyDelay < 0.f) {
			keyDelay = 0.f;
		} else {
			mouseMask = 0;
		}
	}
	if(mouseMask) keyDelay = 250.f;
/*
	if(move) {
		pos = mousePos + mouseRel;
		if(pos[0] < 0.f) {
			pos[0] = 0.f;
			mouseRel = pos - mousePos;
		}
		if(pos[1] < 0.f) {
			pos[1] = 1.f;
			mouseRel = pos - mousePos;
		}
		if(pos[0]+size[0] > SCRWIDTH) {
			pos[0] = SCRWIDTH-size[0];
			mouseRel = pos - mousePos;
		}
		if(pos[1]+size[1] > SCRHEIGHT) {
			pos[1] = SCRHEIGHT-size[1];
			mouseRel = pos - mousePos;
		}

		if(!mouseMask || mousePos[1] > pos[1]+WINDOW_TITLE_SIZE_Y) {
			move = false;
		}
	}*/

	
	if(mousePos[0] > pos[0] && mousePos[1] > pos[1] && mousePos[0] < pos[0]+size[0] && mousePos[1] < pos[1]+size[1]) {
		active = true;
		if(!pulledUp)
			game->getUI()->captureMouse();
	} else active = false;

	if(active && mouseMask) {
		if(mousePos[1] < pos[1]+WINDOW_TITLE_SIZE_Y) {
			if(pulledUp) {
				pulledUp = false;
				pullDownAnim = true;
			} else {
				pullUpAnim = true;
				pullDownAnim = false;
			}
			/*move = true;
			mouseRel = pos - mousePos;*/
		}
	}

	if(active && mousePos[1] < pos[1]+WINDOW_TITLE_SIZE_Y) hover = true;
	else hover = false;

	if(pullDownAnim) {
		pullDownPos -= frameDelta*size[1]*0.01f;
		active = false;
		if(pullDownPos < 0.f) {
			pullDownPos = 0.f;
			pullDownAnim = false;
		}
	} else if(pullUpAnim) {
		pullDownPos += frameDelta*size[1]*0.01f;
		active = false;
		if(pullDownPos > size[1]) {
			pullDownPos = size[1];
			pullUpAnim = false;
			pulledUp = true;
		}
	}

	drawBG();
	GameNode::update(frameDelta);
	drawSelf();
	
}

UIWidget* UIWindow::attachWidget(UIWidget* widget) {
	game->addNode(widget, this);
	return widget;
}

void UIWindow::drawBG() {
	Tmpl8::Surface* screen = game->getScreen();
	bg->CopyTo(screen, pos[0], pos[1]-pullDownPos);
	screen->FilledBox(pos[0], pos[1], pos[0]+size[0], pos[1]+size[1]-pullDownPos, WINDOW_BORDER_COLOR);
}

void UIWindow::drawSelf() {
	Tmpl8::Surface* screen = game->getScreen();
	if(hover) title_bg->Clear(WINDOW_TITLE_HOVER_COLOR);
	else title_bg->Clear(WINDOW_TITLE_COLOR);
	title_bg->CopyTo(screen, pos[0], pos[1]);
	title_surf->DrawScaled(pos[0]+5.f, pos[1]+5.f, 1024, 40, screen);	
}

vec2 UIWindow::getPosition() {
	if(pulledUp || pullDownAnim || pullUpAnim) return vec2(pos[0], pos[1]-pullDownPos);
	return pos;
}

vec2 UIWindow::getSize() {
	return size;
}

bool UIWindow::isActive() {
	return active;
}

// UILabel

UILabel::UILabel(Tmpl8::Game* game_n, UIWindow* window_n, float pos_y_n, char* text_n) {
	game = game_n;
	window = window_n;
	pos_y = pos_y_n;
	text = text_n;
	Tmpl8::Surface* tmp = new Tmpl8::Surface(256, 20);
	tmp->InitCharset();
	tmp->Clear(0);
	tmp->Print(text, 0, 0, LABEL_TEXT_COLOR);
	text_surf = new Tmpl8::Sprite(tmp, 1);
	state = LS_NONE;
	bg = new Tmpl8::Surface(window->getSize()[0]-LABEL_MARGIN*2.f, LABEL_SIZE_Y);
	bg->Clear(state);
}

UILabel::~UILabel() {

}

void UILabel::update(float frameDelta) {
	drawSelf();	
}

void UILabel::drawSelf() {
	vec2 w_pos = window->getPosition();
	vec2 draw_pos = w_pos;
	draw_pos[1] += pos_y;
	draw_pos[0] += LABEL_MARGIN;

	bg->CopyTo(game->getScreen(), draw_pos[0], draw_pos[1]);
	text_surf->DrawScaled(draw_pos[0]+LABEL_TEXT_PADDING, draw_pos[1]+LABEL_TEXT_PADDING*2.f, 512, 40, game->getScreen());
	game->getScreen()->FilledBox(draw_pos[0], draw_pos[1], draw_pos[0]+window->getSize()[0]-LABEL_MARGIN*2.f, draw_pos[1]+LABEL_SIZE_Y, WINDOW_BORDER_COLOR);
}

void UILabel::setState(UILabelState state_n) {
	state = state_n;
	bg->Clear(state);
}

// UIMenu

UIMenu::UIMenu(Tmpl8::Game* game_n, UIWindow* window_n, float pos_y_n, char* var) : callback(var) {
	game = game_n;
	window = window_n;
	pos_y = pos_y_n;

	*callback = 0;
	selected = 0;
	items_n = 0;
}

UIMenu::~UIMenu() {

}

void UIMenu::update(float frameDelta) {
	GameNode::update(frameDelta);

	// goto still considered harmful.. except when you want to do cleanup from several exit points, then it's not a sin
	if(!window->isActive())
		goto UI_MENU_CLEAR_STATE;

	vec2 mousePos = game->getMousePos();
	vec2 abspos = window->getPosition();
	abspos[1] += pos_y;
	vec2 abssize(window->getSize()[0], LABEL_SIZE_Y*items_n);

	if(!(mousePos[0] > abspos[0] && mousePos[1] > abspos[1] && mousePos[0] < abspos[0]+abssize[0] && mousePos[1] < abspos[1]+abssize[1]))
		goto UI_MENU_CLEAR_STATE;

	int hover = (mousePos[1] - abspos[1])/LABEL_SIZE_Y;
	int i = 0;

	int pressed = game->getMouseMask();
	for(std::list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		if(i == hover) {
			if(pressed) {
				selected = i;
				*callback = i;
				(*x)->setState(LS_SELECTED);
			} else {
				(*x)->setState(LS_HOVER);	
			}
		} else {
			if(i != *callback)
				(*x)->setState(LS_NONE);
			else
				(*x)->setState(LS_SELECTED);
		}
		i++;
	}

	return;

	UI_MENU_CLEAR_STATE:
	int q = 0;
	for(std::list<UILabel*>::iterator x = labels.begin(); x != labels.end(); x++) {
		if(q == *callback)
			(*x)->setState(LS_SELECTED);
		else
			(*x)->setState(LS_NONE);
		q++;
	}
}

void UIMenu::addItem(char* text) {
	UILabel* label = (UILabel*)window->attachWidget(new UILabel(game, window, pos_y+LABEL_SIZE_Y*items_n, text));
	labels.push_back(label);
	items_n++;
}

// UIMgr

UIMgr::UIMgr(Tmpl8::Game* game_n) {
	game = game_n;
	game->addNode(this);
	mouseFree = true;
	nextWindowPos = 2.f;
}

UIMgr::~UIMgr() {

}

void UIMgr::update(float frameDelta) {
	mouseFree = true;
	GameNode::update(frameDelta);
}

UIWindow* UIMgr::createWindow(vec2 init_pos, vec2 size, char* title, bool hidable) {
	float pos_x = nextWindowPos;
	nextWindowPos += size[0];
	return (UIWindow*)game->addNode(new UIWindow(game, vec2(pos_x, 0.f), size, title, hidable), this); 
}

UILabel* UIMgr::createLabel(UIWindow* window, float pos_y, char* text) {
	return (UILabel*)window->attachWidget(new UILabel(game, window, pos_y, text));
}

UIMenu* UIMgr::createMenu(UIWindow* window, float pos_y, char* var) {
	return (UIMenu*)window->attachWidget(new UIMenu(game, window, pos_y, var));
}

bool UIMgr::isMouseFree() {
	return mouseFree;
}

void UIMgr::captureMouse() {
	mouseFree = false;
}