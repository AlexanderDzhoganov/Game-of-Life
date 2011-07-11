#include <string> 
#include <map>
#include <list>
#include <iostream>
#include "vec2.h"
#include "cvar.h"
#include "cvar_console.h"


CVarConsole::CVarConsole() {
	funcDef = NULL;
	std::string welcome("--------------\nCVarConsole v0.1\n--------------\n");
	out_buffer.push_back(welcome);
}

CVarConsole::~CVarConsole() {

}

bool CVarConsole::parseInput(char* input) {
	if(strlen(input) == 0) return true;

	if(parseAssignment(input)) return true;
	if(parseCommand(input)) return true;
	if(parseIdentifier(input)) return true;

	return false;
}

bool CVarConsole::parseIdentifier(char* input) {
	input = stripVar(input);
	if(!input) return false;

	bool vcount = cmap.DoesExist(input);
	if(!vcount) {
		char buffer[STR_BUFFER];
		sprintf(buffer, "Identifier not found \"%s\"\n", input);
		std::string out(buffer);
		out_buffer.push_back(out);
		return false;
	} else {
		CType type = cmap.GetType(input);
		if(type == CType_int) {
			CInt tmp(input);
			char buffer[STR_BUFFER];
			sprintf(buffer, "(int) %s = %i (ref: %i)\n", input, *tmp, cmap.GetRef(&tmp)-1);
			std::string out(buffer);
			out_buffer.push_back(out);
		} else if(type == CType_float) {
			CFloat tmp(input);
			char buffer[STR_BUFFER];
			sprintf(buffer, "(float) %s = %f (ref: %i)\n", input, *tmp, cmap.GetRef(&tmp)-1);
			std::string out(buffer);
			out_buffer.push_back(out);
		} else if(type == CType_vec2) {
			CVec2 tmp(input);
			char buffer[STR_BUFFER];
			sprintf(buffer, "(vec2) %s = [%f, %f] (ref: %i)\n", input, (*tmp)[0], (*tmp)[1], cmap.GetRef(&tmp)-1);
			std::string out(buffer);
			out_buffer.push_back(out);
		}
	}
	return true;
}

bool CVarConsole::parseCommand(char* input) {
	char command[STR_BUFFER];
	char args[ARG_BUFFER][STR_BUFFER];
	int args_ptr = 0;

	char* token = strtok(input, " ");
	if(!token) return true;
	strcpy(command, token);

	while(token != NULL) {
		strcpy(args[args_ptr], token);
		args_ptr++;
		token = strtok(NULL, " ");
	}

	if(!strcmp(command, "list")) {
		std::list<std::string> names = cmap.GetVars();
		for(std::list<std::string>::iterator x = names.begin(); x != names.end(); x++) {
			char* name = (char*)(*x).c_str();
			CType type = cmap.GetType(name);
			if(type == CType_int) {
				CInt tmp(name);
				char buffer[STR_BUFFER];
				sprintf(buffer, "(int) %s = %i (ref: %i)\n", name, *tmp, cmap.GetRef(&tmp)-1);
				std::string out(buffer);
				out_buffer.push_back(out);
			} else if(type == CType_float) {
				CFloat tmp(name);
				char buffer[STR_BUFFER];
				sprintf(buffer, "(float) %s = %f (ref: %i)\n", name, *tmp, cmap.GetRef(&tmp)-1);
				std::string out(buffer);
				out_buffer.push_back(out);
			} else if(type == CType_vec2) {
				CVec2 tmp(name);
				char buffer[STR_BUFFER];
				sprintf(buffer, "(vec2) %s = [%f, %f] (ref: %i)\n", name, (*tmp)[0], (*tmp)[1], cmap.GetRef(&tmp)-1);
				std::string out(buffer);
				out_buffer.push_back(out);
			}
		}
		return true;
	}
	
	if(!strcmp(command, "declare")) {
		if(args_ptr < 3) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Insufficient arguments to %s", command);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}
		char* var = args[2];
		var = stripVar(var);
		if(!var) return false;

		char* strtype = args[1];

		bool vcount = cmap.DoesExist(var);
		if(vcount) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Identifier \"%s\" already exists", var);
			std::string out(buffer);
			out_buffer.push_back(out);
		} else {
			CType type = cmap.GetTypeFromStr(strtype);
			if(type == CType_int) {
				CInt tmp(var, true, true);
			} else if(type == CType_float) {
				CFloat tmp(var, true, true);
			} else if(type == CType_vec2) {
				CVec2 tmp(var, true, true);
			}
		}

		return true;
	}

	if(funcDef) {
		for(std::list<char*>::iterator x = funcDef->exports.begin(); x != funcDef->exports.end(); x++) {
			if(!strcmp(command, *x)) {
				return funcDef->Call(*x, this, args, args_ptr);
			}
		}
	}
/*	if(!strcmp(command, "call")) {
		if(args_ptr < 2) {
			print("Insufficient arguments to call");
			return true;
		}
		if(!funcDef) {
			print("FuncDef class not passed - call unavailable");
			return true;
		}

		if(!funcDef->Call(args[1], this)) {
			print("Call function not found within FuncDef");
			return true;
		} else {
			return true;
		}
		return false;
	}*/

	if(!strcmp(command, "about")) {
		char buffer[STR_BUFFER];
		sprintf(buffer, "CVarConsole v0.1 - written by Alexander Dzhoganov");
		print(buffer);
		return true;
	}

	if(!strcmp(command, "help")) {
		char buffer[STR_BUFFER*2];
		sprintf(buffer, "available commands:\nlist declare quit about\nadd sub mul div\n");
		if(funcDef) {
			for(std::list<char*>::iterator x = funcDef->exports.begin(); x != funcDef->exports.end(); x++) {
				sprintf(buffer, "%s%s ", buffer, *x);
			}
		}
		print(buffer);
		return true;
	}

	if(!strcmp(command, "quit")) {
		exit(0);
	}

	// arithmetics

	if(!strcmp(command, "add")) {
		if(args_ptr < 3) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Insufficient arguments to %s", command);
			print(buffer);
			return true;
		}
		char* var_a = args[1];
		char* var_b = args[2];

		char* parsed_a = stripVar(var_a);
		if(parsed_a) {
			var_a = parsed_a;
		} else return false;

		char* parsed_b = stripVar(var_b);
		char vardata_b[128];

		if(parsed_b) {
			var_b = parsed_b;
			if(!cmap.DoesExist(var_b)) {
				char buffer[STR_BUFFER];
				sprintf(buffer, "Invalid identifier \"%s\"", var_b);
				std::string out(buffer);
				out_buffer.push_back(out);
				return true;
			}

			CType type_b = cmap.GetType(var_b);
			if(type_b == CType_int) {
				CInt tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%i", *tmp);
			} else if(type_b == CType_float) {
				CFloat tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%f", *tmp);
			}

			var_b = vardata_b;
		}

		if(!cmap.DoesExist(var_a)) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Invalid identifier \"%s\"", var_a);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}

		CType type = cmap.GetType(var_a);
		if(type == CType_int) {
			CInt tmp(var_a);
			*tmp += atoi(var_b);
			return true;
		} else if(type == CType_float) {
			CFloat tmp(var_a);
			*tmp += atof(var_b);
			return true;
		}
	}

	if(!strcmp(command, "sub")) {
		if(args_ptr < 3) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Insufficient arguments to %s", command);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}
		char* var_a = args[1];
		char* var_b = args[2];

		char* parsed_a = stripVar(var_a);
		if(parsed_a) {
			var_a = parsed_a;
		} else return false;

		char* parsed_b = stripVar(var_b);
		char vardata_b[128];

		if(parsed_b) {
			var_b = parsed_b;
			if(!cmap.DoesExist(var_b)) {
				char buffer[STR_BUFFER];
				sprintf(buffer, "Invalid identifier \"%s\"", var_b);
				std::string out(buffer);
				out_buffer.push_back(out);
				return true;
			}

			CType type_b = cmap.GetType(var_b);
			if(type_b == CType_int) {
				CInt tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%i", *tmp);
			} else if(type_b == CType_float) {
				CFloat tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%f", *tmp);
			}

			var_b = vardata_b;
		}

		if(!cmap.DoesExist(var_a)) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Invalid identifier \"%s\"", var_a);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}

		CType type = cmap.GetType(var_a);
		if(type == CType_int) {
			CInt tmp(var_a);
			*tmp -= atoi(var_b);
			return true;
		} else if(type == CType_float) {
			CFloat tmp(var_a);
			*tmp -= atof(var_b);
			return true;
		}
	}

	if(!strcmp(command, "mul")) {
		if(args_ptr < 3) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Insufficient arguments to %s", command);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}
		char* var_a = args[1];
		char* var_b = args[2];

		char* parsed_a = stripVar(var_a);
		if(parsed_a) {
			var_a = parsed_a;
		} else return false;

		char* parsed_b = stripVar(var_b);
		char vardata_b[128];

		if(parsed_b) {
			var_b = parsed_b;
			if(!cmap.DoesExist(var_b)) {
				char buffer[STR_BUFFER];
				sprintf(buffer, "Invalid identifier \"%s\"", var_b);
				std::string out(buffer);
				out_buffer.push_back(out);
				return true;
			}

			CType type_b = cmap.GetType(var_b);
			if(type_b == CType_int) {
				CInt tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%i", *tmp);
			} else if(type_b == CType_float) {
				CFloat tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%f", *tmp);
			}

			var_b = vardata_b;
		}

		if(!cmap.DoesExist(var_a)) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Invalid identifier \"%s\"", var_a);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}

		CType type = cmap.GetType(var_a);
		if(type == CType_int) {
			CInt tmp(var_a);
			*tmp *= atoi(var_b);
			return true;
		} else if(type == CType_float) {
			CFloat tmp(var_a);
			*tmp *= atof(var_b);
			return true;
		}
	}

	if(!strcmp(command, "div")) {
		if(args_ptr < 3) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Insufficient arguments to %s", command);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}
		char* var_a = args[1];
		char* var_b = args[2];

		char* parsed_a = stripVar(var_a);
		if(parsed_a) {
			var_a = parsed_a;
		} else return false;

		char* parsed_b = stripVar(var_b);
		char vardata_b[128];

		if(parsed_b) {
			var_b = parsed_b;
			if(!cmap.DoesExist(var_b)) {
				char buffer[STR_BUFFER];
				sprintf(buffer, "Invalid identifier \"%s\"", var_b);
				std::string out(buffer);
				out_buffer.push_back(out);
				return true;
			}

			CType type_b = cmap.GetType(var_b);
			if(type_b == CType_int) {
				CInt tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%i", *tmp);
			} else if(type_b == CType_float) {
				CFloat tmp(var_b, IMMUTABLE);
				sprintf(vardata_b, "%f", *tmp);
			}

			var_b = vardata_b;
		}

		if(!cmap.DoesExist(var_a)) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Invalid identifier \"%s\"", var_a);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}

		CType type = cmap.GetType(var_a);
		if(type == CType_int) {
			CInt tmp(var_a);
			*tmp /= atoi(var_b);
			return true;
		} else if(type == CType_float) {
			CFloat tmp(var_a);
			*tmp /= atof(var_b);
			return true;
		}
	}

	return false;
}

bool CVarConsole::parseAssignment(char* input) {
	bool isAssignment = false;
	for(int x=0; x<strlen(input); x++) {
		if(input[x] == '=') {
			isAssignment = true;
			break;
		}
	}
	if(!isAssignment) return false;

	char* token = strtok(input, "=");
	bool LHset = false;
	char LH[STR_BUFFER];
	char RH[STR_BUFFER];

	while(token != NULL) {
		if(!LHset) {
			strcpy(LH, token);
			LHset = true;
		}
		else strcpy(RH, token);
		token = strtok(NULL, "=");
	}

	char* strippedLH = stripVar(strip(LH));
	char* strippedRH = strip(RH);
	char* varname = stripVar(strippedRH);
	char vardata[STR_BUFFER];

	if(varname) {
		if(!cmap.DoesExist(varname)) {
			char buffer[STR_BUFFER];
			sprintf(buffer, "Identifier not found \"%s\"\n", varname);
			std::string out(buffer);
			out_buffer.push_back(out);
			return true;
		}
		
		if(cmap.GetType(varname) == CType_int) {
			CInt tmp(varname);
			sprintf(vardata, "%i", *tmp); 
		} else if(cmap.GetType(varname) == CType_float) {
			CFloat tmp(varname);
			sprintf(vardata, "%i", *tmp); 
		} if(cmap.GetType(varname) == CType_vec2) {
			CVec2 tmp(varname);
			sprintf(vardata, "%f, %f", (*tmp)[0], (*tmp)[1]); 
		}
		strippedRH = vardata;
	}

	bool vcount = cmap.DoesExist(strippedLH);
	if(!vcount) {
		char buffer[128];
		sprintf(buffer, "Identifier not found \"%s\"\n", strip(LH));
		std::string out(buffer);
		out_buffer.push_back(out);
	} else {
		CType type = cmap.GetType(strippedLH);
		if(type == CType_int) {
			CInt tmp(strippedLH);
			*tmp = atoi(strippedRH);
		} else if(type == CType_float) {
			CFloat tmp(strippedLH);
			*tmp = atof(strippedRH);
		} else if(type == CType_vec2) {
			CVec2 tmp(strippedLH);
			char* x = strtok(strippedRH, ",");
			char* y = strtok(NULL, ",");
			if(!x || !y) {
				char buffer[STR_BUFFER];
				sprintf(buffer, "Not enough arguments for vec2 assignment", strippedLH);
				std::string out(buffer);
				out_buffer.push_back(out);
				return true;
			}
			(*tmp)[0] = atof(x);
			(*tmp)[1] = atof(y);
		}
	}

	return true;
}

std::string CVarConsole::outputNextLine() {
	std::string line = out_buffer.front();
	out_buffer.pop_front();
	return line;
}

bool CVarConsole::hasOutput() {
	if(out_buffer.size() != 0) return true;
	return false;
}

void CVarConsole::print(char* str) {
		std::string out(str);
		out_buffer.push_back(out);
}

void CVarConsole::passFuncDefClass(CFuncDef* func) {
	funcDef = func;
}