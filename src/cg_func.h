#ifndef __CG_FUNC
#define __CG_FUNC

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <zip.h>
#include <zlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "cg_struct.h"

void ErrorHandler(
	int err_num, char const* message,
	struct cg_param* cg_parameter );

unsigned int LengthUntilChar(
	char const* str, char ch );

// param values must not contain spaces
void GetParamFromStr(
	char const* param_str, struct cg_param* param_struct );
	
xmlNodePtr FormulaNodeHandler(
	xmlNodePtr formula_node, FILE* output_file,
	struct cg_param* cg_parameter, struct interval_single_set* oper );

int CharBelongToSet(
	char ch, struct interval_single_set* set );
	
int StrBelongToList(
	char* str, char** list,
	unsigned int count );
	
xmlNodePtr VarNameHandler(
	xmlNodePtr formula_node, char*** var_name_list,
	unsigned int* var_list_size, unsigned int* var_name_count,
	struct interval_single_set* begin_char_set, struct interval_single_set* body_char_set );
	
xmlNodePtr FindNodeByPrefixName(
	xmlNodePtr start_node, char const* prefix,
	char const* name );
	
xmlNodePtr NextSiblingParent(
	xmlNodePtr node );
	
int FuncNameCheck(
	xmlChar* text, struct interval_single_set* begin_char_set,
	struct interval_single_set* body_char_set );
	
char* GetReturnVarName(
	xmlNodePtr math_para );

#endif