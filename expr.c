//表达式分析

#include "cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int *sp;

void expr_init(void) {
	static int is_init = 0;
	if(!is_init) {
		sp = (int*)malloc(MAXSIZE * sizeof(int));
	}
}

int const_null(void) {
	if(tki == Null) {
		next();
		return 0;
	} else { printf("error27!\n"); exit(-1); }
}

int const_str(void) {
	if(tki == STR) {
		int offset = sgetstr(tks) -> offset;
		next();
		return offset;
	} else { printf("error28!\n"); exit(-1); }
}

void expr_arr(int env, Type *type, int offset) {
	if(env == GLO) {
		memset(data + offset, 0, type -> count);
		if(strcmp(tks, "{")) { printf("error29!\n"); exit(-1); }
		next();
		if(strcmp(tks, "}")) {
			int count = 0;
			while(1) {
				count++;
				if(type -> rely -> base == INT) {
					data[offset] = const_expr("");
				} else if(type -> rely -> base == CHAR) {
					data[offset] = const_expr("");
				} else if(type -> rely -> base == PTR) {
					if(tki == STR) {
						if(type -> rely -> rely -> base == CHAR) {
							data[offset] = const_str();
						} else { printf("error30!\n"); exit(-1); }
					} else data[offset] = const_null();
					//data[offset] = const_null();
				} else if(type -> rely -> base == ARR) {
					expr_arr(GLO, type -> rely, offset);
				}
				if(!strcmp(tks, "}")) {
					break;
				} else if(!strcmp(tks, ",")) {
					offset += typesize(type -> rely);
					next();
				} else { printf("error31!\n"); exit(-1); }
			}
			if(count > type -> count) { printf("error32!\n"); exit(-1); }
		}
		next();
	} else if(env == LOC) {
		if(strcmp(tks, "{")) { printf("error33!\n"); exit(-1); }
		next();
		if(strcmp(tks, "}")) {
			int count = 0;
			while(1) {
				count++;
				if(type -> rely -> base == INT) {
					*e++ = AL; *e++ = offset;
					*e++ = PUSH; *e++ = AX;
					if(type -> rely != expr("").type) { printf("error34!\n"); exit(-1); }
					*e++ = ASS;
				} else if(type -> rely -> base == CHAR) {
					*e++ = AL; *e++ = offset;
					*e++ = PUSH; *e++ = AX;
					if(type -> rely != expr("").type) { printf("error35!\n"); exit(-1); }
					*e++ = ASS;
				} else if(type -> rely -> base == PTR) {
					*e++ = AL; *e++ = offset;
					*e++ = PUSH; *e++ = AX;
					if(type -> rely != expr("").type) { printf("error36!\n"); exit(-1); }
					*e++ = ASS;
				} else if(type -> rely -> base == ARR) {
					expr_arr(LOC, type -> rely, offset);
				}
				if(!strcmp(tks, "}")) {
					break;
				} else if(!strcmp(tks, ",")) {
					offset += typesize(type -> rely);
					next();
				} else { printf("error37!\n"); exit(-1); }
			}
			if(count > type -> count) { printf("error38!\n"); exit(-1); }
		}
		next();
	}
}

static int lev(char *opr) { //优先级越高lev越大，其他符号lev为0
	char *oprs[] = {
		")", "]",
		"", "&&", "||", "!",
		"", "==", "!=",
		"", ">", "<", ">=", "<=",
		"", "+", "-",
		"", "*", "/", "%",
		"", "=",
		"", "ref", "&",
		"", "(", "["
	};
	int lev = 1;
	for(int i = 0; i < sizeof(oprs) / sizeof(*oprs); i++) {
		if(!strcmp(oprs[i], opr)) {
			return lev;
		} else if(!strcmp(oprs[i], "")) {
			lev++;
		}
	}
	return 0; //其他符号
}

int const_expr(char *last_opr) {
	if(tki == INT) {
		*sp = atoi(tks);
		next();
	} else if(tki == CHAR) {
		*sp = tks[0];
	} else if(!strcmp(tks, "(")) {
		next();
		const_expr(")");
		if(strcmp(tks, ")")) { printf("error39!\n"); exit(-1); } //"("无法匹配到")"
		next();
	} else if(!strcmp(tks, "!")) {
		next();
		const_expr("!");
		*sp = !*sp;
	} else { printf("error40!\n"); exit(-1); }
	
	while(lev(tks) > lev(last_opr)) {
		char *opr = tks;
		sp++;
		next();
		const_expr(opr);
		int opr2 = *sp;
		int opr1 = *--sp;
		if (!strcmp(opr, "+")) *sp = opr1 + opr2;
		else if(!strcmp(opr, "-")) *sp = opr1 - opr2;
		else if(!strcmp(opr, "*")) *sp = opr1 * opr2;
		else if(!strcmp(opr, "/")) *sp = opr1 / opr2;
		else if(!strcmp(opr, "%")) *sp = opr1 % opr2;
		else if(!strcmp(opr, "==")) *sp = opr1 == opr2;
		else if(!strcmp(opr, ">")) *sp = opr1 > opr2;
		else if(!strcmp(opr, "<")) *sp = opr1 < opr2;
		else if(!strcmp(opr, "!=")) *sp = opr1 == opr2;
		else if(!strcmp(opr, ">=")) *sp = opr1 >= opr2;
		else if(!strcmp(opr, "<=")) *sp = opr1 <= opr2;
		else if(!strcmp(opr, "&&")) *sp = opr1 && opr2;
		else if(!strcmp(opr, "||")) *sp = opr1 || opr2;
		else { printf("error41!\n"); exit(-1); }
	}
	return *sp;
}

void expr_check(Type *type1, Type *type2, char *opr) {
	if(!strcmp(opr, "=")) {
		if(type1 -> base == INT || type1 -> base == CHAR) {
			if(type2 -> base == INT);
			else if(type2 -> base == CHAR);
			else { printf("error42!\n"); exit(-1); }
		} else if(type1 -> base == PTR) {
			if(type1 == type2);
			else if(type2 -> base == NUL);
			else if(type2 -> base == PTR && type2 -> rely -> base == VOID);
			else { printf("error43!\n"); exit(-1); }
		}
	} else if(!strcmp(opr, "+") || !strcmp(opr, "-")) {
		if(type1 -> base == INT || type1 -> base == CHAR || type1 -> base == PTR || type1 -> base == ARR) {
			if(type2 -> base == INT);
			else if(type2 -> base == CHAR);
			else { printf("error44!\n"); exit(-1); }
		} else { printf("error45!\n"); exit(-1); }
	} else if(!strcmp(opr, "*") || !strcmp(opr, "/") || !strcmp(opr, "%")) {
		if(type1 -> base == INT || type1 -> base == CHAR) {
			if(type2 -> base == INT);
			else if(type2 -> base == CHAR);
			else { printf("error46!\n"); exit(-1); }
		} else { printf("error47!\n"); exit(-1); }
	} else if(!strcmp(opr, "==") || !strcmp(opr, ">") || !strcmp(opr, "<") || !strcmp(opr, "!=") || !strcmp(opr, ">=") || !strcmp(opr, "<=")) {
		if(type1 -> base == INT || type1 -> base == CHAR) {
			if(type2 -> base == INT);
			else if(type2 -> base == CHAR);
			else { printf("error48!\n"); exit(-1); }
		} else if(type1 -> base == PTR) {
			if(type1 == type2);
			else if(type2 -> base == NUL);
			else { printf("error49!\n"); exit(-1); }
		} else if(type1 -> base == NUL) {
			if(type1 == type2);
			else if(type2 -> base == PTR);
			else { printf("error50!\n"); exit(-1); }
		} else { printf("error51!\n"); exit(-1); }
	} else if(!strcmp(opr, "&&") || !strcmp(opr, "||")) {
		if(type1 -> base == INT || type1 -> base == CHAR) {
			if(type2 -> base == INT);
			else if(type2 -> base == CHAR);
			else { printf("error52!\n"); exit(-1); }
		} else { printf("error53!\n"); exit(-1); }
	} else { printf("error54!\n"); exit(-1); }
}

Er expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	Er er = {NULL, 0, 1};
	if(tki == INT) {
		er.type = deriv_type(INT, NULL, 0);
		*e++ = SET; *e++ = AX; *e++ = atoi(tks);
		next();
	} else if(tki == CHAR) {
		er.type = deriv_type(CHAR, NULL, 0);
		*e++ = SET; *e++ = AX; *e++ = tks[0];
		next();
	} else if(tki == NUL) {
		er.type = deriv_type(NUL, NULL, 0);
		*e++ = SET; *e++ = AX; *e++ = 0;
		next();
	} else if(tki == STR) {
		Id *this_id = sgetstr(tks);
		*e++ = AG; *e++ = this_id -> offset;
		er.type = this_id -> type;
		if(strcmp(last_opr, "&")) er.type = deriv_type(PTR, er.type -> rely, 0);
		er.is_lvalue = 1;
		next();
	} else if(tki == ID) {
		Id *this_id = getid(tks);
		er.type = this_id -> type;
		*e++ = this_id -> class == GLO ? AG: AL; *e++ = this_id -> offset;
		if(er.type -> base == INT || er.type -> base == CHAR || er.type -> base == PTR) {
			*e++ = VAL;
			er.is_const = 0;
		}
		er.is_lvalue = 1;
		next();
	} else if(!strcmp(tks, "(")) {
		next();
		er.type = expr(")").type;
		if(strcmp(tks, ")")) { printf("error55!\n"); exit(-1); }
		next();
	} else if(!strcmp(tks, "*")) {
		next();
		er.type = expr("ref").type;
		if(er.type -> base != PTR) { printf("error56!\n"); exit(-1); }
		er.type = er.type -> rely;
		if(er.type -> base == INT || er.type -> base == CHAR || er.type -> base == PTR) {
			*e++ = VAL;
			er.is_const = 0;
		}
		er.is_lvalue = 1;
	} else if(!strcmp(tks, "&")) {
		next();
		Er _er = expr("&");
		if(!_er.is_lvalue) { printf("error57!\n"); exit(-1); }
		if(_er.type -> base == INT || _er.type -> base == PTR) e--;
		er.type = deriv_type(PTR, _er.type, 0);
	} else if(!strcmp(tks, "!")) {
		next();
		er.type = expr("!").type;
		if(er.type -> base != INT) er.type = deriv_type(INT, NULL, 0);
		*e++ = NOT;
	} else { printf("error58!\n"); exit(-1); }
	
	while(lev(tks) > lev(last_opr)) {
		*e++ = PUSH; *e++ = AX;
		if(!strcmp(tks, "=")) {
			next();
			e -= 3; *e++ = PUSH; *e++ = AX;
			expr_check(er.type, expr("").type, "=");
			if(!er.is_const) *e++ = ASS; else { printf("error59!\n"); exit(-1); }
		} else if(!strcmp(tks, "(")) {
			next();
			int argc = 0;
			if(er.type -> base == PTR) er.type = er.type -> rely; //将函数指针转化为函数
			if(er.type -> base != FUN && er.type -> base != API) { printf("error60!\n"); exit(-1); }
			if(strcmp(tks, ")")) {
				while(1) {
					if(argc > er.type -> count) { printf("error61!\n"); exit(-1); } //参数过多
					Type *argtype = expr(")").type;
					if(argtype -> base == FUN) argtype = deriv_type(PTR, argtype, 0);
					else if(argtype -> base == ARR) argtype = deriv_type(PTR, argtype -> rely, 0);
					expr_check((er.type -> argtyls)[argc], argtype, "="); //参数类型检查
					*e++ = PUSH; *e++ = AX;
					argc++;
					if(!strcmp(tks, ")")) break;
					else if(!strcmp(tks, ",")) next();
					else { printf("error62!\n"); exit(-1); }
				}
			}
			next();
			if(argc < er.type -> count) { printf("error63!\n"); exit(-1); } //参数过少
			if(er.type -> base == FUN) {
				*e++ = SET; *e++ = AX; int *_e = e++; //set next ip
				*e++ = PUSH; *e++ = AX;
				*e++ = CALL; *e++ = argc;
				*_e = e - emit;
				*e++ = DEC; *e++ = SP; *e++ = argc + 1;
			} else if(er.type -> base == API) {
				*e++ = CAPI; *e++ = argc;
				*e++ = DEC; *e++ = SP; *e++ = argc + 1;
			}
			er.type = er.type -> rely;
		} else if(!strcmp(tks, "[")) {
			next();
			if(er.type -> base == PTR || er.type -> base == ARR) er.type = er.type -> rely;
			else { printf("error64!\n"); exit(-1); }
			Type *type = expr("]").type;
			if(type -> base != INT && type -> base != CHAR) { printf("error65!\n"); exit(-1); }
			*e++ = PUSH; *e++ = AX;
			*e++ = SET; *e++ = AX; *e++ = typesize(er.type);
			*e++ = MUL;
			*e++ = ADD;
			if(er.type -> base == INT || er.type -> base == CHAR || er.type -> base == PTR) {
				*e++ = VAL;
				er.is_const = 0;
			}
			er.is_lvalue = 1;
			next();
		} else if(!strcmp(tks, "+")) {
			next();
			expr_check(er.type, expr("").type, "+");
			if(er.type -> base == INT) {
				*e++ = ADD;
			} else if(er.type -> base == PTR || er.type -> base == ARR) {
				*e++ = PUSH; *e++ = AX;
				*e++ = SET; *e++ = AX; *e++ = typesize(er.type -> rely);
				*e++ = MUL;
				*e++ = ADD;
				if(er.type -> base == ARR) er.type = deriv_type(PTR, er.type -> rely, 0);
			} else { printf("error66!\n"); exit(-1); }
		} else if(!strcmp(tks, "-")) {
			next();
			expr_check(er.type, expr("").type, "-");
			if(er.type -> base == INT) {
				*e++ = SUB;
			} else if(er.type -> base == PTR || er.type -> base == ARR) {
				*e++ = PUSH; *e++ = AX;
				*e++ = SET; *e++ = AX; *e++ = typesize(er.type -> rely);
				*e++ = MUL;
				*e++ = SUB;
				if(er.type -> base == ARR) er.type = deriv_type(PTR, er.type -> rely, 0);
			} else { printf("error67!\n"); exit(-1); }
		} else {
			char *opr = tks;
			next();
			expr_check(er.type, expr(opr).type, opr);
			er.type = deriv_type(INT, NULL, 0);
			if(!strcmp(opr, "*")) *e++ = MUL;
			else if(!strcmp(opr, "/")) *e++ = DIV;
			else if(!strcmp(opr, "%")) *e++ = MOD;
			else if(!strcmp(opr, "==")) *e++ = EQ;
			else if(!strcmp(opr, ">")) *e++ = GT;
			else if(!strcmp(opr, "<")) *e++ = LT;
			else if(!strcmp(opr, "!=")) { *e++ = EQ; *e++ = NOT; }
			else if(!strcmp(opr, ">=")) { *e++ = LT; *e++ = NOT; }
			else if(!strcmp(opr, "<=")) { *e++ = GT; *e++ = NOT; }
			else if(!strcmp(opr, "&&")) *e++ = AND;
			else if(!strcmp(opr, "||")) *e++ = OR;
			else { printf("error68!\n"); exit(-1); }
		}
	}
	return er;
}
