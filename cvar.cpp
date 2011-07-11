#include <string>
#include <map>
#include <list>
#include <iostream>
#include "vec2.h"
#include "cvar.h"

CMap::CMap() {
	integers = new int[CMAP_MAX_PER_TYPE];
	integers_mask = new bool[CMAP_MAX_PER_TYPE];
	integers_ref = new int[CMAP_MAX_PER_TYPE];
	floats = new float[CMAP_MAX_PER_TYPE];
	floats_mask = new bool[CMAP_MAX_PER_TYPE];
	floats_ref = new int[CMAP_MAX_PER_TYPE];
	vec2s = new vec2[CMAP_MAX_PER_TYPE];
	vec2s_mask = new bool[CMAP_MAX_PER_TYPE];
	vec2s_ref = new int[CMAP_MAX_PER_TYPE];
	
	for(int x=0; x<CMAP_MAX_PER_TYPE; x++) {
		integers[x] = 0;
		integers_mask[x] = 0;
		integers_ref[x] = 0;
		floats[x] = 0;
		floats_mask[x] = 0;
		floats_ref[x] = 0;
		vec2s_mask[x] = 0;
		vec2s_ref[x] = 0;
	}
}

CMap::~CMap() {
	delete [] integers;
	delete [] integers_mask;
	delete [] integers_ref;
	delete [] floats;
	delete [] floats_mask;
	delete [] floats_ref;
	delete [] vec2s;
	delete [] vec2s_mask;
	delete [] vec2s_ref;
}

void CMap::BindVar(CVar* var) {
	int index = -1;

	switch(var->type) {
	case CType_int:
		if(db.count(var->name) != 0) {
			index = db[var->name];
			var->index = index;
			integers_ref[index]++;
			break;
		}

		for(int x=0; x<CMAP_MAX_PER_TYPE; x++) {
			if(!integers_mask[x]) {
				index = x;
				break;
			}
		}
		if(index == -1) throw CVarOutOfBounds();
		var->index = index;
		integers_mask[index] = 1;
		integers_ref[index]++;
		db[var->name] = index;
		db_type[var->name] = CType_int;
		break;
	case CType_float:
		if(db.count(var->name) != 0) {
			index = db[var->name];
			var->index = index;
			floats_ref[index]++;
			break;
		}

		for(int x=0; x<CMAP_MAX_PER_TYPE; x++) {
			if(!floats_mask[x]) {
				index = x;
				break;
			}
		}
		if(index == -1) throw CVarOutOfBounds();
		var->index = index;
		floats_mask[index] = 1;
		floats_ref[index]++;
		db[var->name] = index;
		db_type[var->name] = CType_float;
		break;
	case CType_vec2:
		if(db.count(var->name) != 0) {
			index = db[var->name];
			var->index = index;
			vec2s_ref[index]++;
			break;
		}

		for(int x=0; x<CMAP_MAX_PER_TYPE; x++) {
			if(!vec2s_mask[x]) {
				index = x;
				break;
			}
		}
		if(index == -1) throw CVarOutOfBounds();
		var->index = index;
		vec2s_mask[index] = 1;
		vec2s_ref[index]++;
		db[var->name] = index;
		db_type[var->name] = CType_vec2;
		break;
	}
}

void CMap::UnbindVar(CVar* var) {
	switch(var->type) {
	case CType_int:
		integers_ref[var->index]--;
		if(integers_ref[var->index] <= 0) {
			integers_mask[var->index] = 0;
			db.erase(var->name);
			db_type.erase(var->name);
		}
		break;
	case CType_float:
		floats_ref[var->index]--;
		if(floats_ref[var->index] <= 0) {
			floats_mask[var->index] = 0;
			db.erase(var->name);
			db_type.erase(var->name);
		}
		break;
	case CType_vec2:
		vec2s_ref[var->index]--;
		if(vec2s_ref[var->index] <= 0) {
			vec2s_mask[var->index] = 0;
			db.erase(var->name);
			db_type.erase(var->name);
		}
		break;
	}
}

int& CMap::GetInt(int index) {
	return integers[index];
}

float& CMap::GetFloat(int index) {
	return floats[index];
}

vec2& CMap::GetVec2(int index) {
	return vec2s[index];
}

bool CMap::DoesExist(char* name) { 
	if(!name) return false;
	if(db.find(name) != db.end()) return true;
	return false;
}

CType CMap::GetType(char* name) {
	return db_type[name];
}

std::list<std::string> CMap::GetVars() {
	std::list<std::string> vars;

	for(std::map<std::string, int>::const_iterator x = db.begin(); x != db.end(); x++) {
		vars.push_back(x->first);
	}
	
	return vars;
}

CType CMap::GetTypeFromStr(char* str) {
	if(!strcmp(str, "int")) return CType_int;
	if(!strcmp(str, "float")) return CType_float;
	if(!strcmp(str, "vec2")) return CType_vec2;
	return CType_int;
}

int CMap::GetRef(CVar* var) {
	switch(var->type) {
	case CType_int:
		return integers_ref[var->index];
	case CType_float:
		return	floats_ref[var->index];
	case CType_vec2:
		return vec2s_ref[var->index];
	};
	return -1;
}