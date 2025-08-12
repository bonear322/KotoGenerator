#define _CRT_SECURE_NO_WARNINGS

#include "cg_struct.h"
#include "cg_error.h"
#include "cg_func.h"

extern int cg_error;

void ErrorHandler(
	int err_num, char const* message,
	struct cg_param* cg_parameter )
{
	if ( message == NULL )
	{
		switch ( cg_error )
		{
			case ( BAD_DOCX_FILE_STRUCT ):
			{
				MessageBox(NULL, "Неправильная структура входного docx-файла", "Ошибка", MB_ICONERROR | MB_OK);
				break;
			}
			default:
				MessageBox(NULL, "Ошибка", "Ошибка", MB_ICONERROR | MB_OK);
		}
	}
	else
	{
		MessageBox(NULL, message, "Ошибка", MB_ICONERROR | MB_OK);
		free( message );
	}
	
	if (cg_parameter != NULL)
	{
		char* outputf_path = (char*)calloc(strlen(cg_parameter->output) + sizeof("\\output.c"), sizeof(char));
		if (outputf_path != NULL)
		{
			strcat(outputf_path, cg_parameter->output);
			strcat(outputf_path, "\\output.c");
			fclose(cg_parameter->output_file);
			cg_parameter->output_file = NULL;
			remove(outputf_path);
			free(outputf_path);
		}
	}
	
	cg_error = CG_OK;
};

unsigned int LengthUntilChar(
	char const* str, char ch )
{
	unsigned int count = 0;
	
	for ( unsigned int i = 0; ; i++ )
	{
		if ( str[ i ] == ch )
		{
			count++;
			break;
		}
	
		if ( str[ i ] == '\0' )
		{
			count++;
			cg_error = NO_REQ_CHAR;
			return count;
		}
	
		count++;
	}
	
	cg_error = CG_OK;
	return count;
};

// param values must not contain spaces
void GetParamFromStr(
	char const* param_str, struct cg_param* param_struct )
{
	param_struct->input = NULL;
	param_struct->output = NULL;
	
	size_t param_str_len = strlen( param_str ) + 1;
	char const* param_str_end = strchr( param_str, '\0' );
	char* param_val = NULL;
	unsigned int buf_len[ 2 ];
	__int64 param_val_pos[ 2 ];
				
	if (
	!( param_val = strstr( param_str, "-inputf " ) ) ||
	( ( param_val += sizeof( "-inputf " ) / sizeof( char ) - 1 ) >= param_str_end ) ||
	!( buf_len[ 0 ] = LengthUntilChar( param_val, ' ' ) ) ||
	!( param_val_pos[ 0 ] = param_val - param_str ) )
	{ cg_error = NO_REQ_PARAM; return; }
	
	if (
	!( param_val = strstr( param_str, "-outputf " ) ) ||
	( ( param_val += sizeof( "-outputf " ) / sizeof( char ) - 1 ) >= param_str_end ) ||
	!( buf_len[ 1 ] = LengthUntilChar( param_val, ' ' ) ) ||
	!( param_val_pos[ 1 ] = param_val - param_str ) )
	{ cg_error = NO_REQ_PARAM; return; }
	
	if (
	!( param_val = strstr( param_str, "-var_type " ) ) ||
	( ( param_val += sizeof( "-var_type " ) / sizeof( char ) - 1 ) >= param_str_end ) )
	{ cg_error = NO_REQ_PARAM; return; }
	
	if ( strncmp( param_val, "float", 5 ) == 0 )
	{ param_struct->type = CG_FLOAT; }
	else if ( strncmp( param_val, "double", 6 ) == 0 )
	{ param_struct->type = CG_DOUBLE; }
	else
	{ cg_error = NO_REQ_PARAM; return; }
	
	if (
	( param_val = strstr( param_str, "-word_trig_notat" ) ) &&
	( ( param_val[ sizeof( "-word_trig_notat" ) / sizeof( char ) - 1 ] == ' ' ) ||
	  ( param_val[ sizeof( "-word_trig_notat" ) / sizeof( char ) - 1 ] == '\0' ) ) )
	{ param_struct->word_trig_notat = 1; }
	else
	{ param_struct->word_trig_notat = 0; }

	if (
	!( param_val = strstr( param_str, "-line_elem_count " ) ) ||
	( ( param_val += sizeof( "-line_elem_count " ) / sizeof( char ) - 1 ) >= param_str_end ) )
	{ cg_error = NO_REQ_PARAM; return; }
	
	if ( sscanf( param_val, "%u", &param_struct->line_elem_count ) < 1 )
	{ cg_error = NO_REQ_PARAM; return; }
	
	if (
	!( param_struct->input = ( char* )calloc( buf_len[ 0 ], sizeof( char ) ) ) ||
	!( param_struct->output = ( char* )calloc( buf_len[ 1 ], sizeof( char ) ) ) )
	{
		cg_error = BAD_ALLOC;
		if ( param_struct->input != NULL  )
		{ free( param_struct->input ); }
		if ( param_struct->output != NULL  )
		{ free( param_struct->output ); }
		
		return;
	}
	
	strncat( param_struct->input, &param_str[ param_val_pos[ 0 ] ], buf_len[ 0 ] - 1 );
	param_struct->input[ buf_len[ 0 ] - 1 ] = '\0';
	strncat( param_struct->output, &param_str[ param_val_pos[ 1 ] ], buf_len[ 1 ] - 1 );
	param_struct->output[ buf_len[ 1 ] - 1 ] = '\0';
	
	cg_error = CG_OK;
	return;
};

xmlNodePtr FormulaNodeHandler(
	xmlNodePtr formula_node, FILE* output_file,
	struct cg_param* cg_parameter, struct interval_single_set* oper )
{
	int math_tag_flag = 1;
	static int trig_sup_flag = 0;
	
	if ( formula_node == NULL )
	{
		cg_error = BAD_XML_NODE;
		return NULL;
	}
	
	if (
	( formula_node->ns == NULL ) ||
	( formula_node->ns->prefix == NULL ) ||
	( strcmp( formula_node->ns->prefix, "m" ) != 0 ) )
	{
		math_tag_flag = 0;
		goto NextNode;
	}
	
	if ( strcmp( formula_node->name, "t" ) == 0 )
	{
		xmlChar* content = xmlNodeGetContent( formula_node );
		size_t content_len = strlen( content );
		/*fprintf( output_file, content );*/
		
		for ( size_t i = 0; i < content_len; i++ )
		{
			if ( content[ i ] == '=' )
			{
				fprintf( output_file, " = " ); continue;
			}
			if ( CharBelongToSet( content[ i ], oper ) == 1 )
			{
				// number check
				if ( i != 0 )
				{
					if (
					( content[ i - 1 ] >= 48 ) && // 0-9
					( content[ i - 1 ] <= 57 ) )
					{
						int num_part_of_name_flag = 0;
						int num_point_flag = 0;
						
						for ( size_t i_2 = i; i_2 > 0 ; i_2-- )
						{
							if (
							( ( content[ i_2 - 1 ] >= 65 ) && ( content[ i_2 - 1 ] <= 90 ) ) || // A-Z
							( ( content[ i_2 - 1 ] >= 97 ) && ( content[ i_2 - 1 ] <= 122 ) ) || // a-z
							( content[ i_2 - 1 ] == '_' ) )
							{
								num_part_of_name_flag = 1;
								break;
							}
							if ( content[ i_2 - 1] == '.' )
							{
								num_point_flag = 1;
								break;
							}
							if (
							( content[ i_2 - 1 ] == '+' ) || ( content[ i_2 - 1 ] == '-' ) ||
							( content[ i_2 - 1 ] == '*' ) || ( content[ i_2 - 1 ] == '/' ) ||
							( content[ i_2 - 1 ] == ':' ) )
							{
								break;
							}
						}
						
						if ( num_part_of_name_flag == 0 )
						{
							if ( cg_parameter->type == CG_FLOAT )
							{
								if ( num_point_flag == 0 )
								{
									fprintf( output_file, ".0f" );
								}
								else
								{
									fprintf( output_file, "f" );
								}
							}
							else if ( cg_parameter->type == CG_DOUBLE )
							{
								if ( num_point_flag == 0 )
								{
									fprintf( output_file, ".0" );
								}
							}
							else
							{}
						}
					}
				}
				
				if ( content[ i ] == ':' )
				{
					fputc( '/', output_file );
				}
				else
				{
					fputc( content[ i ], output_file );
				}
				
				// line separating
				if ( cg_parameter->line_elem_count > 0 )
				{
					( cg_parameter->line_oper_count )++;
					if (cg_parameter->line_oper_count == cg_parameter->line_elem_count )
					{
						fprintf( output_file, "\n" );
						fprintf( output_file, cg_parameter->line_offset_str );
						cg_parameter->line_oper_count = 0;
					}
				}
			}
			else
			{
				fputc( content[ i ], output_file );
			}
		}
		
		// number check
		if (
		( content[ content_len - 1 ] >= 48 ) && // 0-9
		( content[ content_len - 1 ] <= 57 ) )
		{
			int num_part_of_name_flag = 0;
			int num_point_flag = 0;
			
			for ( size_t i_2 = content_len; i_2 > 0 ; i_2-- )
			{
				if (
				( ( content[ i_2 - 1 ] >= 65 ) && ( content[ i_2 - 1 ] <= 90 ) ) || // A-Z
				( ( content[ i_2 -1 ] >= 97 ) && ( content[ i_2 - 1 ] <= 122 ) ) || // a-z
				( content[ i_2 - 1 ] == '_' ) )
				{
					num_part_of_name_flag = 1;
					break;
				}
				if ( content[ i_2 - 1 ] == '.' )
				{
					num_point_flag = 1;
					break;
				}
				if (
				( content[ i_2 - 1 ] == '+' ) || ( content[ i_2 - 1 ] == '-' ) ||
				( content[ i_2 - 1 ] == '*' ) || ( content[ i_2 - 1 ] == '/' ) ||
				( content[ i_2 - 1 ] == ':' ) )
				{
					break;
				}
			}
			
			if ( num_part_of_name_flag == 0 )
			{
				if ( cg_parameter->type == CG_FLOAT )
				{
					if ( num_point_flag == 0 )
					{
						fprintf( output_file, ".0f" );
					}
					else
					{
						fprintf( output_file, "f" );
					}
				}
				else if ( cg_parameter->type == CG_DOUBLE )
				{
					if ( num_point_flag == 0 )
					{
						fprintf( output_file, ".0" );
					}
				}
				else
				{}
			}
		}
		
		xmlFree( content );
	}
	else if ( strcmp( formula_node->name, "e" ) == 0 )
	{
		
	}
	else if ( strcmp( formula_node->name, "d" ) == 0 )
	{
		if ( formula_node->children->children->next == NULL )
		{
			if (
			(
			( formula_node->parent != NULL ) &&
			( strcmp( formula_node->parent->name, "e" ) == 0 ) &&
			( strcmp( formula_node->parent->parent->name, "sSup" ) == 0 ) ) ||
			(
			( formula_node->parent != NULL ) &&
			( strcmp( formula_node->parent->name, "e" ) == 0 ) &&
			( strcmp( formula_node->parent->parent->name, "func" ) == 0 ) ) )
			{
				//fprintf( output_file, "(" );
			}
			else
			{
				fprintf(output_file, "(");
			}
			
			cg_error = CG_OK;
			return formula_node->children->next;
		}
		else
		{
			xmlChar* beg_chr = xmlNodeGetContent( formula_node->children->children->properties );
			xmlChar* end_chr = xmlNodeGetContent( formula_node->children->children->next->properties );
			
			if (
			( strcmp( beg_chr, "|" ) == 0 ) &&
			( strcmp( end_chr, "|" ) == 0 ) )
			{
				if ( cg_parameter->type == CG_FLOAT )
				{
					fprintf( output_file, "fabsf(" );
				}
				else if ( cg_parameter->type == CG_DOUBLE )
				{
					fprintf( output_file, "fabs(" );
				}
				else
				{}
			
				xmlFree( beg_chr );
				xmlFree( end_chr );
				cg_error = CG_OK;
				return formula_node->children->next;
			}
			else
			{
				xmlFree( beg_chr );
				xmlFree( end_chr );
			}
		}
		
		cg_error = BAD_XML_NODE;
		return NULL;
	}
	else if ( strcmp( formula_node->name, "f" ) == 0 )
	{
		if (
		( formula_node->parent == NULL ) ||
		( formula_node->next == NULL ) )
		{}
		else
		{
			fprintf( output_file, "(" );
		}
	}
	else if ( strcmp( formula_node->name, "num" ) == 0 )
	{
		fprintf( output_file, "(" );
	}
	else if ( strcmp( formula_node->name, "den" ) == 0 )
	{
		fprintf( output_file, "(" );
	}
	else if ( strcmp( formula_node->name, "sSup" ) == 0 )
	{
		if ( cg_parameter->type == CG_DOUBLE )
		{
			fprintf( output_file, "pow(" );
		}
		else if ( cg_parameter->type == CG_FLOAT )
		{
			fprintf( output_file, "powf(" );
		}
		else
		{  }
	
		cg_error = CG_OK;
		return formula_node->children->next;
	}
	else if ( strcmp( formula_node->name, "sup" ) == 0 )
	{
		fprintf( output_file, ", " );
		
		cg_error = CG_OK;
		return formula_node->children;
	}
	else if ( strcmp( formula_node->name, "sub" ) == 0 )
	{
		xmlChar* content = xmlNodeGetContent( formula_node->children->children->/*m:t*/next );
				
		fprintf( output_file, "_%s", content );
		xmlFree( content );
		
		formula_node = formula_node->children->children->/*m:t*/next;
	}
	else if ( strcmp( formula_node->name, "func" ) == 0 )
	{
		xmlNodePtr func_name_text_node;
		xmlChar* func_name;
		char inverse_func_flag = 0;
		
		if ( cg_parameter->word_trig_notat != 0 )
		{
			if ( strcmp( formula_node->children->/*m:fName*/next->children->name, "sSup" ) != 0 )
			{ inverse_func_flag = 0; }
			else
			{ inverse_func_flag = 1; }
			
			if ( inverse_func_flag == 0 )
			{
				func_name_text_node = formula_node->children->/*m:fName*/next;
				func_name_text_node = func_name_text_node->children->children->next->/*m:t*/next;
			}
			else
			{
				func_name_text_node = formula_node->children->/*m:fName*/next;
				func_name_text_node = func_name_text_node->children->children->/*m:e*/next;
				func_name_text_node = func_name_text_node->children->children->next->/*m:t*/next;
			}
			func_name = xmlNodeGetContent( func_name_text_node );
		}
		else
		{
			if ( strcmp( formula_node->children->/*m:fName*/next->children->name, "sSup" ) == 0 )
			{
				trig_sup_flag = 1;
				
				if ( cg_parameter->type == CG_FLOAT )
				{
					fprintf( output_file, "powf(" );
				}
				else if ( cg_parameter->type == CG_DOUBLE )
				{
					fprintf( output_file, "pow(" );
				}
				else
				{}
			
				func_name_text_node = formula_node->children->/*m:fName*/next;
				func_name_text_node = func_name_text_node->children->children->/*m:e*/next;
				func_name_text_node = func_name_text_node->children->children->next->/*m:t*/next;
			}
			else
			{
				func_name_text_node = formula_node->children->/*m:fName*/next;
				func_name_text_node = func_name_text_node->children->children->next->/*m:t*/next;
			}
			func_name = xmlNodeGetContent( func_name_text_node );
		}
		
		if ( strcmp( func_name, "sin" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "sin(" ); }
				else
				{ fprintf( output_file, "asin(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "sinf(" ); }
				else
				{ fprintf( output_file, "asinf(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "cos" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "cos(" ); }
				else
				{ fprintf( output_file, "acos(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "cosf(" ); }
				else
				{ fprintf( output_file, "acosf(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "tan" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "tan(" ); }
				else
				{ fprintf( output_file, "atan(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "tanf(" ); }
				else
				{ fprintf( output_file, "atanf(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "csc" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0/sin(" ); }
				else
				{ fprintf( output_file, "asin(1.0/(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0f/sinf(" ); }
				else
				{ fprintf( output_file, "asinf(1.0f/(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "sec" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0/cos(" ); }
				else
				{ fprintf( output_file, "acos(1.0/(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0f/cosf(" ); }
				else
				{ fprintf( output_file, "acosf(1.0f/(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "cot" ) == 0 )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0/tan(" ); }
				else
				{ fprintf( output_file, "atan(1.0/(" ); }
			}
			else if ( cg_parameter->type == CG_FLOAT )
			{
				if ( inverse_func_flag == 0 )
				{ fprintf( output_file, "(1.0f/tanf(" ); }
				else
				{ fprintf( output_file, "atanf(1.0f/(" ); }
			}
			else
			{  }
		}
		else if ( strcmp( func_name, "arcsin" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "asinf(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "asin(" );
			}
			else
			{}
		}
		else if ( strcmp( func_name, "arccos" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "acosf(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "acos(" );
			}
			else
			{}
		}
		else if ( strcmp( func_name, "arctan" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "atanf(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "atan(" );
			}
			else
			{}
		}
		else if ( strcmp( func_name, "arccsc" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "asinf(1.0f/(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "asin(1.0/(" );
			}
			else
			{}
		}
		else if ( strcmp( func_name, "arcsec" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "acosf(1.0f/(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "acos(1.0/(" );
			}
			else
			{}
		}
		else if ( strcmp( func_name, "arccot" ) == 0 )
		{
			if ( cg_parameter->type == CG_FLOAT )
			{
				fprintf( output_file, "atanf(1.0f/(" );
			}
			else if ( cg_parameter->type == CG_DOUBLE )
			{
				fprintf( output_file, "atan(1.0/(" );
			}
			else
			{}
		}
		
		xmlFree( func_name );
		
		cg_error = CG_OK;
		return formula_node->children->next->/*m:e*/next;
	}
	else if ( strcmp( formula_node->name, "rad" ) == 0 )
	{
		if ( formula_node->children->/*m:deg*/next->children == NULL )
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{ fprintf( output_file, "sqrt(" ); }
			else if ( cg_parameter->type == CG_FLOAT )
			{ fprintf( output_file, "sqrtf(" ); }
			else
			{  }
		
			cg_error = CG_OK;
			return formula_node->last->children;
		}
		else
		{
			if ( cg_parameter->type == CG_DOUBLE )
			{ fprintf( output_file, "pow(" ); }
			else if ( cg_parameter->type == CG_FLOAT )
			{ fprintf( output_file, "powf(" ); }
			else
			{  }
		
			cg_error = CG_OK;
			return formula_node->last->children;
		}
	}
	else if ( strcmp( formula_node->name, "deg" ) == 0 )
	{
		if ( cg_parameter->type == CG_FLOAT )
		{ fprintf(output_file, "1.0f/("); }
		else if ( cg_parameter-> type == CG_DOUBLE )
		{ fprintf(output_file, "1.0/("); }
		else
		{  }
		
		cg_error = CG_OK;
		return formula_node->children;
	}
	
	// to next node
	NextNode:
	if ( formula_node->children != NULL )
	{
		cg_error = CG_OK;
		return formula_node->children;
	}
	else if ( formula_node->next != NULL )
	{
		cg_error = CG_OK;
		return formula_node->next;
	}
	else
	{
		CloseNode:
		while ( formula_node != NULL )
		{
			if (
			( formula_node->ns == NULL ) ||
			( formula_node->ns->prefix == NULL ) ||
			( strcmp( formula_node->ns->prefix, "m" ) != 0 ) )
			{
				math_tag_flag = 0;
				goto ParentNext;
			}
			
			if ( strcmp( formula_node->name, "d" ) == 0 )
			{
				if ( formula_node->children->children->next == NULL )
				{
					if (
					(
					( formula_node->parent != NULL ) &&
					( strcmp( formula_node->parent->name, "e" ) == 0) &&
					( strcmp( formula_node->parent->parent->name, "sSup" ) == 0 ) ) ||
					(
					( formula_node->parent != NULL ) &&
					( strcmp(formula_node->parent->name, "e" ) == 0) &&
					( strcmp(formula_node->parent->parent->name, "func" ) == 0 ) ) )
					{
						//fprintf( output_file, ")" );
					}
					else
					{
						fprintf( output_file, ")" );
					}
				}
				else
				{
					fprintf( output_file, ")" );
				}
			}
			else if ( strcmp( formula_node->name, "f" ) == 0 )
			{
				if (
				( formula_node->parent == NULL ) ||
				( formula_node->next == NULL ) )
				{}
				else
				{
					fprintf( output_file, ")" );
				}
			}
			else if ( strcmp( formula_node->name, "num" ) == 0 )
			{
				fprintf( output_file, ")" );
				fprintf( output_file, "/" );
				
				// line separating
				if ( cg_parameter->line_elem_count > 0 )
				{
					( cg_parameter->line_oper_count )++;
					if (cg_parameter->line_oper_count == cg_parameter->line_elem_count )
					{
						fprintf( output_file, "\n" );
						fprintf( output_file, cg_parameter->line_offset_str );
						cg_parameter->line_oper_count = 0;
					}
				}
			}
			else if ( strcmp( formula_node->name, "den" ) == 0 )
			{
				fprintf( output_file, ")" );
			}
			else if ( strcmp( formula_node->name, "sSup" ) == 0 )
			{
				fprintf( output_file, ")" );
				
				if (
				( trig_sup_flag == 1 ) &&
				( formula_node->parent != NULL ) &&
				( strcmp( formula_node->parent->name, "fName" ) == 0 ) )
				{
					trig_sup_flag = 0;
					
					formula_node = formula_node->parent->parent;
				}
			}
			else if ( strcmp( formula_node->name, "func" ) == 0 )
			{
				xmlChar* func_name;
				
				if ( strcmp( formula_node->children->/*m:fName*/next->children->name, "sSup" ) != 0 )
				{
					func_name = xmlNodeGetContent( formula_node->children->/*m:fName*/next->children->/*m:t*/last );
				}
				else
				{
					func_name = xmlNodeGetContent( formula_node->children->/*m:fName*/next->children->children->/*m:e*/next->children->/*m:t*/last );
				}
				
				if (
				( strcmp( func_name, "csc" ) == 0 ) ||
				( strcmp( func_name, "sec" ) == 0 ) ||
				( strcmp( func_name, "cot" ) == 0 ) ||
				( strcmp( func_name, "arccsc" ) == 0 ) ||
				( strcmp( func_name, "arcsec" ) == 0 ) ||
				( strcmp( func_name, "arccot" ) == 0 ) )
				{
					fprintf( output_file, "))");
				}
				else
				{
					fprintf( output_file, ")");
				}
				
				xmlFree( func_name );
				
				if ( trig_sup_flag == 1 )
				{
					cg_error = CG_OK;
					return formula_node->children->next->children->last;
				}
			}
			else if ( strcmp( formula_node->name, "rad" ) == 0 )
			{
				if ( formula_node->children->/*m:deg*/next->children == NULL )
				{
					fprintf( output_file, ")");
				}
				else
				{
					fprintf( output_file, ",");
					
					cg_error = CG_OK;
					return formula_node->children->next;
				}
			}
			else if ( strcmp( formula_node->name, "deg" ) == 0 )
			{
				fprintf( output_file, "))");
				
				formula_node = formula_node->parent;
			}
			
			ParentNext:
			if ( formula_node->next != NULL ) 
			{
				cg_error = CG_OK;
				return formula_node->next;
			}
			
			formula_node = formula_node->parent;
		}
		
		cg_error = CG_OK;
		return NULL;
	}
};

int CharBelongToSet(
	char ch, struct interval_single_set* set )
{
	if ( set == NULL )
	{ return -1; }

	for ( unsigned int i = 0, j = 0; i < set->interval_count; i++ )
	{
		if ( ( ch >= set->interval[ j ] ) && ( ch <= set->interval[ j + 1 ] ) )
		{ return 1; }
		j += 2;
	}
	for ( unsigned int i = 0; i < set->single_count; i++ )
	{
		if ( ch == set->single[ i ] )
		{ return 1; }
	}
	
	return 0;
};

int StrBelongToList(
	char* str, char** list,
	unsigned int count )
{
	if ( list == NULL )
	{ return -1; }

	for ( unsigned int i = 0; i < count; i++ )
	{
		if ( strcmp( str, list[ i ] ) == 0 )
		{ return 1; }
	}
	
	return 0;
};

xmlNodePtr VarNameHandler(
	xmlNodePtr formula_node, char*** var_name_list,
	unsigned int* var_list_size, unsigned int* var_name_count,
	struct interval_single_set* begin_char_set, struct interval_single_set* body_char_set )
{
	int math_tag_flag = 1;
	
	if ( formula_node == NULL )
	{
		cg_error = BAD_XML_NODE;
		return NULL;
	}
	
	if (
	( formula_node->ns == NULL ) ||
	( formula_node->ns->prefix == NULL ) ||
	( strcmp( formula_node->ns->prefix, "m" ) != 0 ) )
	{
		math_tag_flag = 0;
		goto NextNode;
	}
	
	if ( strcmp( formula_node->name, "fName" ) == 0 )
	{
		if (strcmp(formula_node->children->name, "sSup") == 0)
		{
			formula_node = formula_node->children->last;
		}
		else
		{
			goto NextParentNext;
		}
	}
	else if ( strcmp( formula_node->name, "t" ) == 0 )
	{
		xmlChar* content = xmlNodeGetContent( formula_node );
		size_t content_len = strlen( content ) /*+ 1*/;
		
		char* var_name_buf = NULL;
		if ( !( var_name_buf = ( char* )malloc( sizeof( char ) * ( content_len + 1 ) ) ) )
		{
			cg_error = BAD_ALLOC;
			return NULL;
		}
		
		for ( unsigned int i = 0, name_char_ind = 0, name_flag = 0; i < content_len + 1; i++ )
		{
			if ( name_flag == 0 )
			{
				if ( CharBelongToSet( content[ i ], begin_char_set ) == 1 )
				{
					var_name_buf[ name_char_ind ] = content[ i ];
					
					name_flag = 1;
					name_char_ind++;
				}
			}
			else
			{
				if ( CharBelongToSet( content[ i ], body_char_set ) == 1 )
				{
					var_name_buf[ name_char_ind ] = content[ i ];
					
					name_char_ind++;
				}
				else
				{
					var_name_buf[ name_char_ind ] = '\0';
					
					if ( StrBelongToList( var_name_buf, *var_name_list, *var_name_count ) == 1 )
					{
						name_flag = 0;
						name_char_ind = 0;
					}
					else
					{
						if ( ( *var_list_size ) < ( *var_name_count ) + 1 )
						{
							char** temp = ( char** )realloc( *var_name_list, sizeof( char* ) * ( ( *var_list_size ) + 4 ) );
							if ( !temp )
							{
								cg_error = BAD_ALLOC;
								return NULL;
							}
							*var_name_list = temp;
							( *var_list_size ) += 4;
						}
						
						if ( !( ( *var_name_list )[ *var_name_count ] = ( char* )calloc( strlen( var_name_buf ) + 1, sizeof( char ) ) ) )
						{
							cg_error = BAD_ALLOC;
							return NULL;
						}
						strcat( ( *var_name_list )[ *var_name_count ], var_name_buf );
						( *var_name_count )++;
						
						name_flag = 0;
						name_char_ind = 0;
					}
				}
			}
		}
		xmlFree( content );
		free( var_name_buf );
	}
	else if ( strcmp( formula_node->name, "sSub" ) == 0 )
	{
		xmlChar* name = xmlNodeGetContent( formula_node->children->/*m:e*/next->children->/*m:t*/last );
		xmlChar* sub_name = xmlNodeGetContent( formula_node->/*m:sup*/last->children->/*m:t*/last );
		
		size_t full_name_len = strlen( name ) + strlen( sub_name ) +/*_*/ 1 /*+ 1*/;
		char* full_name = NULL;
		if ( !( full_name = ( char* )calloc( full_name_len + 1, sizeof( char ) ) ) )
		{
			cg_error = BAD_ALLOC;
			return NULL;
		}
		
		strcat( full_name, name );
		strcat( full_name, "_" );
		strcat( full_name, sub_name );
		
		if ( StrBelongToList( full_name, *var_name_list, *var_name_count ) == 1 )
		{
			
		}
		else
		{
			if ( ( *var_list_size ) < ( *var_name_count ) + 1 )
			{
				char** temp = ( char** )realloc( *var_name_list, sizeof( char* ) * ( ( *var_name_count ) + 4 ) );
				if ( !temp )
				{
					cg_error = BAD_ALLOC;
					return NULL;
				}
				*var_name_list = temp;
				( *var_list_size ) += 4;
			}
			
			if ( !( ( *var_name_list )[ *var_name_count ] = ( char* )calloc( full_name_len + 1, sizeof( char ) ) ) )
			{
				cg_error = BAD_ALLOC;
				return NULL;
			}
			strcat( ( *var_name_list )[ *var_name_count ], full_name );
			( *var_name_count )++;
		}
		
		formula_node = formula_node->/*m:sub*/last->children->/*m:t*/last;
	}
	
	// to next node
	NextNode:
	if ( formula_node->children != NULL )
	{
		cg_error = CG_OK;
		return formula_node->children;
	}
	else /*if (formula_node->next != NULL)*/
	{
		NextParentNext:
		if ( formula_node->next != NULL )
		{
			cg_error = CG_OK;
			return formula_node->next;
		}
		else
		{
			while (formula_node != NULL)
			{
				if (
					(formula_node->parent != NULL) &&
					(formula_node->parent->next != NULL))
				{
					cg_error = CG_OK;
					return formula_node->parent->next;
				}

				formula_node = formula_node->parent;
			}

			cg_error = CG_OK;
			return NULL;
		}
	}
};

xmlNodePtr FindNodeByPrefixName(
	xmlNodePtr start_node, char const* prefix,
	char const* name )
{
	if (
	( start_node == NULL ) ||
	( name == NULL ) )
	{
		cg_error = BAD_XML_NODE;
		return NULL;
	}
	
	xmlNodePtr node = start_node;
	goto NextNode;
	
	while ( node != NULL )
	{
		if ( prefix != NULL )
		{
			if (
			( node->ns == NULL ) ||
			( node->ns->prefix == NULL ) )
			{ goto NextNode; }
			else
			{
				if ( strcmp( node->ns->prefix, prefix ) != 0 )
				{ goto NextNode; }
				else
				{
					if ( strcmp( node->name, name ) != 0 )
					{ goto NextNode; }
					else
					{
						cg_error = CG_OK;
						return node;
					}
				}
			}
		}
		else
		{
			if (
			( node->ns != NULL ) &&
			( node->ns->prefix != NULL ) )
			{ goto NextNode; }
			else
			{
				if ( strcmp( node->name, name ) != 0 )
				{ goto NextNode; }
				else
				{
					cg_error = CG_OK;
					return node;
				}
			}
		}
		
		NextNode:
		if ( node->children != NULL )
		{
			cg_error = CG_OK;
			return node->children;
		}
		else if ( node->next != NULL )
		{
			cg_error = CG_OK;
			return node->next;
		}
		else
		{
			while ( node != NULL )
			{
				if (
				( node->parent != NULL ) &&
				( node->parent->next != NULL ) )
				{
					cg_error = CG_OK;
					return node->parent->next;
				}
				
				node = node->parent;
			}
		}
	}
	
	cg_error = XML_NODE_LISTING_END;
	return NULL;
};

xmlNodePtr NextSiblingParent(
	xmlNodePtr node )
{
	if ( node == NULL )
	{ cg_error = BAD_XML_NODE; return NULL; }

	if ( node->next != NULL )
	{
		cg_error = CG_OK;
		return node->next;
	}
	else
	{
		while ( node != NULL )
		{
			if (
			( node->parent != NULL ) &&
			( node->parent->next != NULL ) )
			{
				cg_error = CG_OK;
				return node->parent->next;
			}
			
			node = node->parent;
		}
	}
	
	cg_error = CG_OK;
	return NULL;
};

int FuncNameCheck(
	xmlChar* text, struct interval_single_set* begin_char_set,
	struct interval_single_set* body_char_set )
{
	if (
	( text == NULL ) ||
	( text[ 0 ] == '\0' ) )
	{ cg_error = NULL_POINTER; return -1; }
	
	size_t text_len = strlen( text );
	if ( text_len < 3 )
	{
		if ( strcmp( text, "==" ) == 0 )
		{ cg_error = WRONG_FUNC_NAME; return -1; }
		else
		{ cg_error = CG_OK; return 0; }
	}
	
	if ( text[ 0 ] == '=' )
	{
		if ( CharBelongToSet( text[ 1 ], begin_char_set ) == 1 )
		{
			for ( size_t i = 2; i < text_len - 1; i++ )
			{
				if ( CharBelongToSet( text[ i ], body_char_set ) == 1 )
				{ continue; }
				else
				{ cg_error = WRONG_FUNC_NAME; return -1; }
			}
			
			if ( text[ text_len - 1 ] == '=' )
			{ cg_error = CG_OK; return 1; }
			else
			{ cg_error = WRONG_FUNC_NAME; return -1; }
		}
		else
		{ cg_error = WRONG_FUNC_NAME; return -1; }
	}
	else
	{ cg_error = CG_OK; return 0; }
};

char* GetReturnVarName(
	xmlNodePtr math_para )
{
	if (
	( math_para == NULL ) ||
	( math_para->ns == NULL ) ||
	( math_para->ns->prefix == NULL ) ||
	( strcmp( math_para->ns->prefix, "m" ) != 0 ) ||
	( strcmp( math_para->name, "oMathPara" ) != 0 ) )
	{ cg_error = BAD_XML_NODE; return NULL; }
	
	char* ret_var_name = NULL;
	size_t ret_var_name_len = 0;
	
	if ( strcmp( math_para->children->children->name, "sSub" ) == 0 )
	{
		xmlChar* name = xmlNodeGetContent( math_para->children->/*m:sSub*/children->children->/*m:e*/next->children->/*m:t*/last );
		xmlChar* sub_name = xmlNodeGetContent( math_para->children->/*m:sSub*/children->/*m:sub*/last->children->/*m:t*/last );
		ret_var_name_len = strlen( name ) + strlen( sub_name ) + /*_*/1;
		if ( ( ret_var_name = ( char* )calloc( ret_var_name_len + 1, sizeof( char ) ) ) == NULL )
		{ xmlFree( name ); xmlFree( sub_name ); cg_error = BAD_ALLOC; return NULL; }
	
		strcat( ret_var_name, name );
		strcat( ret_var_name, "_" );
		strcat( ret_var_name, sub_name );
		
		xmlFree( name ); xmlFree( sub_name ); cg_error = CG_OK; return ret_var_name;
	}
	else if ( strcmp( math_para->children->children->name, "r" ) == 0 )
	{
		xmlChar* name = xmlNodeGetContent( math_para->children->/*m:r*/children->last );
		char* assign_char = strchr( name, '=' );
		*assign_char = '\0';
		ret_var_name_len = strlen( name );
		if ( ( ret_var_name = ( char* )calloc( ret_var_name_len + 1, sizeof( char ) ) ) == NULL )
		{ *assign_char = '='; xmlFree( name ); cg_error = BAD_ALLOC; return NULL; }
		
		strcat( ret_var_name, name );
		
		*assign_char = '='; xmlFree( name ); cg_error = CG_OK; return ret_var_name;
	}
	else
	{ cg_error = CG_FAIL; return NULL; }
};
