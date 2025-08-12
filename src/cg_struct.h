#ifndef __CG_STRUCT
#define __CG_STRUCT

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <zip.h>
#include <zlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

enum var_type
{
	CG_FLOAT,
	CG_DOUBLE
};

enum ret_type
{
	CG_VALUE,
	CG_POINTER
};

enum operator_type
{
	CG_SIN, CG_COS,
	CG_TG, CG_CTG,
	CG_ARCSIN, CG_ARCCOS,
	CG_ARCTG, CG_ARCCTG,
	CG_POW
};

struct cg_operator_prop
{
	enum operator_type oper_t;
	enum var_type var_t;
	enum ret_type ret_t;
	unsigned int ret_pos;
	unsigned int param_num; // ret pointer not include
	unsigned int* param_pos;
	char* name;
	char* include;
};

struct cg_param
{
	char* input;
	char* output;
	FILE* input_file;
	FILE* output_file;
	//int separate;
	enum var_type type;
	char* oper_list;
	unsigned int line_elem_count;
	unsigned int line_oper_count;
	size_t line_offset_str_len;
	char* line_offset_str;
	unsigned int word_trig_notat;
};

struct cg_operator_list
{
	char** cg_name;
	char** name;
	char** include;
};

struct interval_single_set
{
	unsigned int interval_count;
	unsigned int single_count;
	unsigned int* interval;
	char* single;
};

#endif