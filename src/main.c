#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <zip.h>
#include <zlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "cg_struct.h"
#include "cg_global.h"
#include "cg_error.h"
#include "cg_func.h"

int main(
	int argc, char* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	printf( "=KotoGenerator v1.0 by Dendy Kornev=\n" );
	
	if ( argc == 1 )
	{
		if (
		!( param_str = ( char* )malloc( sizeof( char ) * PARAM_BUF_SIZE ) ) ||
		!( param = ( struct cg_param* )calloc( sizeof( struct cg_param ), 1 ) ) )
		{ cg_error = BAD_ALLOC; ErrorHandler( cg_error, NULL, param ); }
		
		printf( "Введите параметры: \n" );
		if ( !fgets( param_str, PARAM_BUF_SIZE, stdin ) )
		{ cg_error = BAD_FGETS; ErrorHandler( cg_error, NULL, param ); }
	
		{
			char* lf_char;
			
			if ( !( lf_char = strchr( param_str, '\n' ) ) )
			{ cg_error = NO_REQ_CHAR; ErrorHandler( cg_error, NULL, param ); }
		
			*lf_char = '\0';
		}
		
		GetParamFromStr( param_str, param );
		if ( cg_error != CG_OK )
		{ ErrorHandler( cg_error, NULL, param ); }
	}
	else
	{
		
	}

	// create output dir
	if ( !CreateDirectory( param->output, NULL ) )
	{
		if ( GetLastError() != ERROR_ALREADY_EXISTS )
		{
			cg_error = BAD_DIRECTORY;
			ErrorHandler(cg_error, NULL, param);
		}
	}
	
	// opening output file
	{
		char* outputf_path = ( char* )calloc( strlen( param->output ) + sizeof( "\\output.c" ), sizeof( char ) );
		if ( outputf_path == NULL )
		{ cg_error = BAD_ALLOC; ErrorHandler( cg_error, NULL, param ); goto exit; }
		strcat( outputf_path, param->output );
		strcat( outputf_path, "\\output.c" );
		
		//output_file = fopen( outputf_path, "w+" );
		if ( ( param->output_file = fopen( outputf_path, "w+" ) ) == NULL )
		{
			cg_error = BAD_FILE;
		
			cg_error_message = ( char* )calloc( sizeof( "Не удалось открыть выходной файл!" ), sizeof( char ) );
			if ( cg_error_message != NULL )
			{
				strcat( cg_error_message, "Не удалось открыть выходной файл!" );
			}
			
			ErrorHandler( cg_error, cg_error_message, param );
			goto exit;
		}
		
		free( outputf_path );
	}
	
	// opening input file
	if (
	!( input_zip = zip_open( param->input, ZIP_RDONLY, &cg_error ) ) ||
	!( input_zfile = zip_fopen( input_zip, "word/document.xml", NULL ) ) )
	{
		//zip_error_t zerr;
		//zip_get_error(input_zip);

		cg_error = BAD_FILE;
		
		cg_error_message = ( char* )calloc( sizeof( "Не удалось открыть входной файл!" ), sizeof( char ) );
		if ( cg_error_message != NULL )
		{
			strcat( cg_error_message, "Не удалось открыть входной файл!" );
		}
		
		ErrorHandler( cg_error, cg_error_message, param );
		goto exit;
	}
	
	zip_stat_init( &input_zfile_stat );
	if( zip_stat( input_zip, "word/document.xml", NULL, &input_zfile_stat ) == -1 )
	{ cg_error = BAD_ZIP; ErrorHandler(cg_error, NULL, param); goto exit; }

	if ( !( input_file_buf = ( char* )malloc( input_zfile_stat.size + 1 ) ) )
	{ cg_error = BAD_ALLOC; ErrorHandler(cg_error, NULL, param); goto exit; }

	if ( zip_fread( input_zfile, input_file_buf, input_zfile_stat.size ) < input_zfile_stat.size )
	{ cg_error = BAD_ZIP; ErrorHandler(cg_error, NULL, param); goto exit; }

	input_file_buf[ input_zfile_stat.size ] = '\0';	
	
	zip_fclose( input_zfile );
	zip_close( input_zip );
	input_zfile = NULL;
	input_zip = NULL;
	
	// xml parser init
	xmlInitParser();
	LIBXML_TEST_VERSION
	
	if (
	!( input_file_xmlTree = xmlParseMemory( input_file_buf, input_zfile_stat.size ) ) ||
	!( input_file_xmlRoot = xmlDocGetRootElement( input_file_xmlTree ) ) )
	{ cg_error = BAD_XML; ErrorHandler(cg_error, NULL, param); goto exit; }
	
	fprintf( param->output_file, "#include <math.h>\n\n");
	
	xmlNodePtr current_node = input_file_xmlRoot;
	
	while ( current_node != NULL )
	{
		if (
		( current_node->ns == NULL ) ||
		( current_node->ns->prefix == NULL ) )
		{ goto NextNode; }
		
		if ( strcmp( current_node->ns->prefix, "w" ) == 0 )
		{
			if ( strcmp( current_node->name, "t" ) == 0 )
			{
				xmlChar* wt_text = NULL;
				if ( ( wt_text = xmlNodeGetContent( current_node ) ) == NULL )
				{ xmlFree( wt_text ); goto NextNode; }
				
				if ( FuncNameCheck( wt_text, &var_name_begin, &var_name_body ) == 0 )
				{ xmlFree( wt_text ); goto NextNode; }
				else if ( cg_error != CG_OK )
				{
					if ( cg_error == WRONG_FUNC_NAME )
					{
						cg_error_message = ( char* )calloc( sizeof( "Некорректное имя функции: " ) + strlen( wt_text ), sizeof( char ) );
						if ( cg_error_message != NULL )
						{
							strcat( cg_error_message, "Некорректное имя функции: " );
							strcat( cg_error_message, wt_text );
						}
						
						ErrorHandler( cg_error, cg_error_message, param );
						xmlFree( wt_text );
						goto exit;
					}
					else
					{ xmlFree( wt_text ); ErrorHandler( cg_error, NULL, param ); goto exit; }
				}
				
				func_name_count++;
				if ( func_name_count != func_formula_count + 1 )
				{ xmlFree( wt_text ); cg_error = BAD_DOCX_FILE_STRUCT; ErrorHandler( cg_error, NULL, param ); goto exit; }
				
				if ( param->type == CG_DOUBLE )
				{ fprintf( param->output_file, "double " ); }
				else if ( param->type == CG_FLOAT )
				{ fprintf( param->output_file, "float " ); }
				else
				{}
				
				for ( unsigned int i = 1; ; i++ )
				{
					if ( wt_text[ i ] == '=' )
					{ break; }
					if ( fputc( wt_text[ i ], param->output_file ) == EOF )
					{ xmlFree( wt_text ); cg_error = C_STD_ERROR; ErrorHandler( cg_error, NULL, param ); goto exit; }
				}
				if ( fputc( '(', param->output_file ) == EOF )
				{ xmlFree( wt_text ); cg_error = C_STD_ERROR; ErrorHandler( cg_error, NULL, param ); goto exit; }
			}
		}
		else if ( strcmp( current_node->ns->prefix, "m" ) == 0 )
		{
			if ( strcmp( current_node->name, "oMathPara" ) == 0 )
			{
				func_formula_count++;
				if ( func_formula_count != func_name_count )
				{ cg_error = BAD_DOCX_FILE_STRUCT; ErrorHandler( cg_error, NULL, param ); goto exit; }
			
				if ( var_name_list != NULL )
				{
					for ( unsigned int i = 0; i < var_name_count; i++ )
					{
						if ( var_name_list[ i ] != NULL )
						{ free( var_name_list[ i ] ); }
					}
					free( var_name_list );
					var_name_list = NULL;
					var_name_list_size = 0;
					var_name_count = 0;
				}
				if ( return_var_name != NULL )
				{ free( return_var_name ); return_var_name = NULL; }
				return_var_name = GetReturnVarName( current_node );
				if ( cg_error != CG_OK )
				{ ErrorHandler( cg_error, NULL, param ); goto exit; }
				
				xmlNodePtr var_name_node = current_node;
				xmlNodePtr next_file_elem_node = NextSiblingParent( current_node );
				if ( cg_error != CG_OK )
				{ ErrorHandler( cg_error, NULL, param ); goto exit; }
			
				while ( var_name_node != next_file_elem_node )
				{
					var_name_node = VarNameHandler( var_name_node, &var_name_list, &var_name_list_size, &var_name_count, &var_name_begin, &var_name_body );
					if ( cg_error != CG_OK )
					{ ErrorHandler( cg_error, NULL, param ); goto exit; }
				}
				
				if ( param->type == CG_FLOAT )
				{
					if ( var_name_count < 3 )
					{ fprintf( param->output_file, "float %s", var_name_list[ 1 ] ); }
					else
					{ fprintf( param->output_file, "float %s,", var_name_list[ 1 ] ); }
				
					for ( unsigned int i = 2; i < var_name_count - 1; i++ )
					{
						fprintf( param->output_file, "float %s,", var_name_list[ i ] );
					}
					
					if ( var_name_count < 3 )
					{ fprintf( param->output_file, ")\n{\n    " ); }
					else
					{ fprintf( param->output_file, "float %s)\n{\n    ", var_name_list[ var_name_count - 1 ] ); }
				}
				else if ( param->type == CG_DOUBLE )
				{
					if ( var_name_count < 3 )
					{ fprintf( param->output_file, "double %s", var_name_list[ 1 ] ); }
					else
					{ fprintf( param->output_file, "double %s,", var_name_list[ 1 ] ); }
				
					for ( unsigned int i = 2; i < var_name_count - 1; i++ )
					{
						fprintf( param->output_file, "double %s,", var_name_list[ i ] );
					}
					
					if ( var_name_count < 3 )
					{ fprintf( param->output_file, ")\n{\n    " ); }
					else
					{ fprintf( param->output_file, "double %s)\n{\n    ", var_name_list[ var_name_count - 1 ] ); }
				}
				else
				{}
			
				if ( param->type == CG_FLOAT )
				{
					fprintf( param->output_file, "float " );
					
					// offset line init
					param->line_offset_str_len = strlen( "    " ) + strlen( "float " ) + strlen( return_var_name ) + /* = */3;
				}
				else if ( param->type == CG_DOUBLE )
				{
					fprintf( param->output_file, "double " );
					
					// offset line init
					param->line_offset_str_len = strlen( "    " ) + strlen( "double " ) + strlen( return_var_name ) + /* = */3;
				}
				
				// offset line init
				if ( ( param->line_offset_str = ( char* )calloc(param->line_offset_str_len + 1, sizeof( char ) ) ) == NULL )
				{ cg_error = BAD_ALLOC; ErrorHandler( cg_error, NULL, param ); goto exit; }
				memset( param->line_offset_str, ' ', param->line_offset_str_len );
				
				while ( current_node != next_file_elem_node )
				{
					current_node = FormulaNodeHandler( current_node, param->output_file, param, &operators );
					if ( cg_error != CG_OK )
					{ ErrorHandler( cg_error, NULL, param ); goto exit; }
				}
				
				fprintf( param->output_file, ";\n\n" );
				fprintf( param->output_file, "    return %s;\n};\n", return_var_name );
				fprintf( param->output_file, "\n" );
				
				free( param->line_offset_str );
				param->line_offset_str = NULL;
				param->line_offset_str_len = 0;
				param->line_oper_count = 0;
				continue;
			}
		}
		else
		{}
	
		NextNode:
		if ( current_node->children != NULL )
		{
			current_node = current_node->children;
		}
		else if ( current_node->next != NULL )
		{
			current_node = current_node->next;
		}
		else
		{
			while (  current_node != NULL )
			{
				if (
				( current_node->parent != NULL ) &&
				( current_node->parent->next != NULL ) )
				{
					current_node = current_node->parent->next;
					break;
				}
				
				 current_node = current_node->parent;
			}
		}
	}
	
	MessageBox( NULL, "Программа успешно завершилась!", "KotoGenerator", MB_OK | MB_ICONINFORMATION );
	exit:
	cg_error_message = NULL;
	//ReleaseCGResources();
	if ( input_file_xmlTree != NULL )
	{
		xmlFreeDoc( input_file_xmlTree );
		input_file_xmlTree = NULL;
	}
	xmlCleanupParser();
	if ( input_file_buf != NULL )
	{
		free( input_file_buf );
		input_file_buf = NULL;
	}
	if ( input_zfile != NULL )
	{
		zip_fclose( input_zfile );
		input_zfile = NULL;
	}
	if ( input_zip != NULL )
	{
		zip_close( input_zip );
		input_zip = NULL;
	}
	if ( ( param != NULL ) && ( param->output_file != NULL ) )
	{
		fclose(param->output_file);
		param->output_file = NULL;
	}
	if ( ( param != NULL ) && ( param->line_offset_str != NULL ) )
	{
		free( param->line_offset_str );
		param->line_offset_str = NULL;
	}
	if ( ( param != NULL ) && ( param->output != NULL ) )
	{
		free( param->output );
		param->output = NULL;
	}
	if ( ( param != NULL ) && ( param->input != NULL ) )
	{
		free( param->input );
		param->input = NULL;
	}
	if ( param != NULL )
	{
		free( param );
		param = NULL;
	}
	if ( param_str != NULL )
	{
		free( param_str );
		param_str = NULL;
	}
	/*
	if (param->output_file != NULL)
	{
		fclose(param->output_file);
		param->output_file = NULL;
	}*/
	return 0;
}