#include "../include/headers.h"

bool FuncDef::Call(char* func, CVarConsole* console, char args[ARG_BUFFER][STR_BUFFER], int args_n) {
		if(!strcmp(func, "print")) {
			for(int x=1; x<args_n; x++) {
				char buffer[256];
				sprintf(buffer, "%s ", args[x]);
				console->print(buffer);
			}
			return true;
		}

		if(!strcmp(func, "clear")) {
			debug->clear();
			return true;
		}

		if(!strcmp(func, "dynlist")) {
			if(args_n < 2) {
				console->print("Insufficient arguments to dynlist");
				return true;
			}
			
			if(!strcmp(args[1], "on")) {
				debug->setDynList(true);
				return true;
			}
			
			if(!strcmp(args[1], "off")) {
				debug->setDynList(false);
				return true;
			}

			if(!strcmp(args[1], "update")) {
				if(args_n < 3) {
					console->print("Insufficient arguments to dynlist");
					return true;
				}
				debug->setDynListUpdateInterval(atof(args[2]));
				return true;
			}
		}

		if(!strcmp(func, "draw")) {
			if(args_n < 2) {
				console->print("Insufficient arguments to draw");
				return true;
			}
			if(!strcmp(args[1], "clear")) {
				debug->clearVectorsToDraw();
				return true;
			}
			
			char* var = stripVar(args[1]);

			if(!cmap.DoesExist(var)) {
				char buffer[256];
				sprintf(buffer, "Invalid identifier \"%s\"", var);
				console->print(buffer);
				return true;
			}
	
			debug->addVectorToDraw(var);
			return true;
		}

		return false;
	}

DebugConsole::DebugConsole(Tmpl8::Game* game_n, int max_lines_num_n, float pos_x_n, float pos_y_n, float size_x_n) {
	game = game_n;
//	game->addNode(this);
	max_lines_num = max_lines_num_n;
	pos_x = pos_x_n;
	pos_y = pos_y_n;
	size_x = size_x_n;
	history_pos = 0;
	console = new CVarConsole();
	fdef = new FuncDef(this);
	console->passFuncDefClass(fdef);

	buffered_input_ptr = 0;
	for(int x=0; x<STR_BUFFER; x++) {
		buffered_input[x] = 0;
	}

	isVisible = false;
	dynList = false;
	dynListTimer = 0.f;
	dynListUpdateInterval = 20.f;
}

DebugConsole::~DebugConsole() {

}

void DebugConsole::getKey(char key) {
	int x = key;

	if(key == 96) {
		// tilde - open/close console
		isVisible = !isVisible;
		return;
	}
	if(key == 0) return;
	if(!isVisible) return;
	if(key == 13) {
		// enter
		bool result = console->parseInput(buffered_input);
		if(!result) {
			addMsg("Syntax error.");
			addMsg("Variables are prefixed with a $ (dollar-sign).");
		}
		buffered_input_ptr = 0;
		buffered_input[0] = 0;
		return;
	}
	if(key == 8 && buffered_input_ptr > 0) {
		// backspace
		buffered_input_ptr--;
		buffered_input[buffered_input_ptr] = 0;
		return;
	} else if(key == 8) return;

	buffered_input[buffered_input_ptr] = key;
	buffered_input[buffered_input_ptr+1] = 0;
	buffered_input_ptr++;
}

void DebugConsole::addMsg(char* text) {
	history.push_back(text);
	if(history.size() > max_lines_num)
		history_pos++;
}

void DebugConsole::update(float frameDelta) {
	for(std::list<char*>::iterator x = vectorsToDraw.begin(); x != vectorsToDraw.end(); x++) {
		CVec2 tmp(*x);
		drawVector(*tmp);
		game->getScreen()->Print(*x, (*tmp)[0]+5, (*tmp)[1]-5, 0xffffffff);
	}

	if(!isVisible) return;
	while(console->hasOutput()) {
		char* output_buffer = new char[256];
		std::string output = console->outputNextLine();
		strcpy(output_buffer, output.c_str());

		char* token = strtok(output_buffer, "\n");
		while(token) {
			char* buffer = new char[256];
			sprintf(buffer, "%s", token);
			addMsg(buffer);
			token = strtok(NULL, "\n");
		}

		delete [] output_buffer;
	}

	drawSelf(game->getScreen());

	if(dynList) {
		dynListTimer += frameDelta;
		if(dynListTimer > dynListUpdateInterval) {
			console->parseCommand("clear");
			console->parseCommand("list");
			dynListTimer -= dynListUpdateInterval;
		}
	}
}

void DebugConsole::drawSelf(Tmpl8::Surface* screen) {
	Tmpl8::Surface* bg = new Tmpl8::Surface(size_x, 20.f*max_lines_num);
	bg->Clear(0x001947);
	bg->CopyTo(screen, pos_x, pos_y);
	delete bg;

	int counter = 0;
	screen->FilledBox(pos_x, pos_y, pos_x+size_x, pos_y+20.f*max_lines_num, 0xffffffff);
	for(std::list<char*>::iterator x = history.begin(); x != history.end(); x++) {
		if(counter >= history_pos) {
			Tmpl8::Surface* tmp = new Tmpl8::Surface(512, 20);
			tmp->InitCharset();
			tmp->Clear(0);
			tmp->Print(*x, 0, 0, 0xffffff);
			Tmpl8::Sprite* tmp_s = new Tmpl8::Sprite(tmp, 1);
			tmp_s->DrawScaled(pos_x+5.f, pos_y+16.f*(counter-history_pos)+5.f, 1024, 40, screen);
			delete tmp_s;
		}
		counter++;
	}
	screen->Line(pos_x, pos_y+max_lines_num*20.f-22.f, pos_x+size_x, pos_y+max_lines_num*20.f-22.f, 0xffffffff);
	
	Tmpl8::Surface* tmp = new Tmpl8::Surface(512, 20);
	tmp->InitCharset();
	tmp->Clear(0);
	tmp->Print(buffered_input, 0, 0, 0xffffff);
	Tmpl8::Sprite* tmp_s = new Tmpl8::Sprite(tmp, 1);
	tmp_s->DrawScaled(pos_x+5.f, pos_y+max_lines_num*20.f-15.f, 1024, 40, screen);
	delete tmp_s;
}

bool DebugConsole::checkCollision(float test_x, float test_y) {
	if(test_x > pos_x && test_x < pos_x+size_x &&\
	   test_y > pos_y && test_y < pos_y+16.f*max_lines_num)
		return true;
	return false;
}

void DebugConsole::offsetHistory(int offset) {
	history_pos += offset;
	if(history_pos < 0) history_pos = 0;
	if(history_pos > history.size()) history_pos = history.size();
}

void DebugConsole::clear() {
	history_pos = 0;
	history.clear();
}

void DebugConsole::addVectorToDraw(char* vec_name) {
	char* buffer = new char[128];
	strcpy(buffer, vec_name);
	vectorsToDraw.push_back(buffer);
}

void DebugConsole::clearVectorsToDraw() {
	vectorsToDraw.clear();
}

void DebugConsole::drawVector(vec2 v) {
	game->getScreen()->Plot(v[0], v[1], 0xff00ff00);
	game->getScreen()->Plot(v[0]+1, v[1], 0xff00ff00);
	game->getScreen()->Plot(v[0]-1, v[1], 0xff00ff00);
	game->getScreen()->Plot(v[0], v[1]+1, 0xff00ff00);
	game->getScreen()->Plot(v[0], v[1]-1, 0xff00ff00);
}

void DebugConsole::setDynList(bool state) {
	dynList = state;
}

void DebugConsole::setDynListUpdateInterval(float interval) {
	dynListUpdateInterval = interval;
}
