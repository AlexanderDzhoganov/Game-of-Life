#include <list>
#include "gamenode.h"

using namespace std;

GameNode::GameNode() {
	children = new list<GameNode*>;
	_markForDeletion = false;
	name = "root";
}

GameNode::~GameNode() {
	deleteChildren();
	delete children;
}

void GameNode::setName(char* newName) {
	if(name) delete name;
	name = new char[strlen(newName)+1];
	for(int x=0; x<strlen(newName); x++) name[x] = newName[x];
	name[strlen(newName)] = 0;
}

char* GameNode::getName() {
	return name;
}

void GameNode::addChild(GameNode* node) {	
	children->push_back(node);
	node->parent = this;
}

void GameNode::deleteChild(GameNode* node) {
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); x++) {
		if(*x == node) {
			delete *x;
			children->erase(x);	
			return;
		}
	}
}

void GameNode::deleteChildren() {	
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); x++) {
		delete (*x);
	}

	children->clear();
}

void GameNode::update(float frameDelta) {
	if(children->size() <= 0) return;
	
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); x++) {
		(*x)->update(frameDelta);
	}
	
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); x++) {
		if((*x)->_markForDeletion) {
			deleteChild(*x);
			return;
		}
	}
}

list<GameNode*>* GameNode::getChildren() {
	return children;	
}

int GameNode::getChildrenNum() {
	return children->size();	
}

int GameNode::getAbsoluteChildrenNum() {
	int n = children->size();
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); x++) {
		n += (*x)->getAbsoluteChildrenNum();
	}
	return n;
}

GameNode* GameNode::findNodeByName(char* search_name) {
	// slow! do not use every frame!
	if(!strcmp(name, search_name)) return this;
	GameNode* tmp;
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); ++x) {
		tmp = (*x)->findNodeByName(search_name);
		if(tmp) return tmp;
	}
	return NULL;
}

GameNode* GameNode::findNodeByType(GameNodeType search_type) {
	// slow! do not use every frame!
	if(type == search_type) return this;
	GameNode* tmp;
	for(list<GameNode*>::iterator x = children->begin(); x != children->end(); ++x) {
		tmp = (*x)->findNodeByType(search_type);
		if(tmp) return tmp;
	}
	return NULL;
}