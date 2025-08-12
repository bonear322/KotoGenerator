#ifndef __CG_GLOBAL
#define __CG_GLOBAL

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <zip.h>
#include <zlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "cg_struct.h"
#include "cg_error.h"

#define PARAM_BUF_SIZE 500

int 			 cg_error;
char* 			 cg_error_message;
char* 			 param_str;
struct cg_param* param;

FILE* output_file;

zip_t* 		input_zip;
zip_file_t* input_zfile;
zip_stat_t  input_zfile_stat;
char* 		comp_input_file_buf;
char* 		input_file_buf;
xmlDocPtr 	input_file_xmlTree;
xmlNodePtr 	input_file_xmlRoot;
xmlChar* 	input_file_xmlNodeContent;
size_t 		input_file_xmlNodeContent_size;
FILE* 		input_file;

unsigned int interval_0[] =
{
	65, 90, // A-Z
	97, 122 // a-z
};
unsigned int interval_1[] =
{
	48, 57, // 0-9
	65, 90, // A-Z
	97, 122 // a-z
};
char single_0[] =
{
	95 // _
};
struct interval_single_set var_name_begin =
{ 2, 1, interval_0, single_0 };
struct interval_single_set var_name_body =
{ 3, 1, interval_1, single_0 };

char** var_name_list;
unsigned int var_name_list_size;
unsigned int var_name_count;
char* return_var_name;

unsigned int func_name_count;
unsigned int func_formula_count;

size_t offset_line_len;
char* offset_line; // no null-term
unsigned int line_oper_count;
char single_1[] =
{ '+', '-', '*', '/', ':' };
struct interval_single_set operators =
{ 0, 5, NULL, single_1 };


#endif