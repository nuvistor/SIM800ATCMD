/*-------------------------------------------------------
at_parser.c
-------------------------------------------------------*/
#include "at_parser.h"
#include <string.h>
#include <stdlib.h>


/*-------------------------------------------------------
parse int value from string
Args:
str = ptr to input string
field_num = number of field to parse(0=парсить с этой позиции, !0=искать разделитель, затем парсить)
max_len = how many bytes in text field to parse
div = divisor between fields 
-------------------------------------------------------*/

uint8_t parse_int( uint8_t* str, uint8_t field_num, uint8_t max_len, uint8_t div, int16_t* result )
{
        static uint8_t parse_str[10];
        uint8_t i, j, c;
        int16_t idx=0; 
        uint8_t max_i;
                
        //перейти к указанному номеру поля
        for( i=0; i<field_num; i++ )
        {
            idx = strpos( &str[idx], div );
            if( idx < 0 )
                    return 0; // no divisor and no such field = error
            idx++;
        };
        
        //копировать поле в строку буфера
        max_i = idx+max_len;
        j = 0;
        parse_str[0] = 0;
        for( i=idx; i<max_i; i++ )
        {
                c = str[idx++];
                if( (c==div) || (c=='\r') || (c=='\n') || (c==0)) 
                        break;
                else
                        parse_str[j++] = c; 
        };
        parse_str[j++] = 0; // terminator
        
        //парсить число
        *result = atoi( parse_str );
        
        return 1;
}

/*-------------------------------------------------------

-------------------------------------------------------*/
signed char ustrpos( char *str, char c )
{
        signed char pos=-1;
         
        while( *str )
        { 
                if( *str == c )
                        break;
                        
                pos++;
        };
        
        return pos;
}

/*-------------------------------------------------------
parse long value from string
Args:
str = ptr to input string
field_num = number of field to parse(0=парсить с этой позиции, !0=искать разделитель, затем парсить)
max_len = how many bytes in text field to parse
div = divisor between fields 
-------------------------------------------------------*/

uint8_t parse_long( uint8_t* str, uint8_t field_num, uint8_t max_len, uint8_t div, int32_t* result )
{
        static uint8_t parse_str[10];
        uint8_t i, j, c;
        int16_t idx=0; 
        uint8_t max_i;
                
        //перейти к указанному номеру поля
        for( i=0; i<field_num; i++ )
        {
            idx += strpos( &str[idx], div ); /// ??? 
            if( idx < 0 )
                    return 0; // no divisor and no such field = error
            idx++;
        };
        
        //копировать поле в строку буфера
        max_i = idx+max_len;
        j = 0;
        parse_str[0] = 0;
        for( i=idx; i<max_i; i++ )
        {
                c = str[idx++];
                if( (c==div) || (c=='\r') || (c=='\n') || (c==0)) 
                        break;
                else
                        parse_str[j++] = c; 
        };
        parse_str[j++] = 0; // terminator
        
        //парсить число
        *result = atol( parse_str );
        
        return 1;
}

/*-------------------------------------------------------
parse quoted string field from response
Arg:
param *src_str = source str
param field_num = number of field to parse(0=парсить с этой позиции, !0=искать разделитель, затем парсить)
param max_len = how many bytes in text field to parse
param delim = delimiter between fields
param *dst_str = result str
Return:
sucess = 1
failure = 0
-------------------------------------------------------*/
uint8_t parse_str_quoted( uint8_t *src_str, uint8_t field_num, uint8_t max_len, uint8_t delim, uint8_t *dst_str )
{
        uint8_t i, c;
        int16_t idx=0; 
        
        //str = <field1>,<field2>,...,<fieldn>         
        //перейти к указанному номеру поля
        for( i=0; i<field_num; i++ )
        {
            idx = strpos( &src_str[idx], delim ); // следующая позиция разделителя поля
            if( idx < 0 )
                    return 0; // no divisor and no such field = error
            idx++;
        };
        //
        if( src_str[idx++] != '\"' ) // двойная кавычка = начало строки
        { 
                return 0; // не строка?
        };
        
        //копировать поле в строку буфера
        dst_str[0] = 0;
        for( i=0; i<max_len; i++ )
        {
                c = src_str[idx++];
                if( (c=='\r') || (c=='\n') || (c==0) || (c=='\"')) // "=конец строки
                        break;
                else
                {
                        *dst_str = c;
                        dst_str++;
                }; 
        };
        *dst_str = 0; // terminator
        //
        
        return 1;
}



/*-------------------------------------------------------
 Args: string RAM, string array Flash, string size in bytes, total items
 Return: (-1) = no string match
 *str, *sample[], items
 если sample[] менье чем *str, считается что строки совпадают 
-------------------------------------------------------*/
int16_t resp_cmp( uint8_t *str, flash uint8_t *resp_sample[], uint8_t items )
{
        uint8_t i, j, c1, c2; // c1=str, c2=resp_sample
        flash uint8_t *p;
        
        for( i=0; i<items; i++ )
        { 
                p = resp_sample[i];
                j = 0;
                do{
                        c1 = str[j++];
                        
                        c2 = *p;
                        ++p;
                        
                        if( c1 != c2 )
                                break;
                        
                }while( c2 != 0 );        
                if( c2 == 0 ) // строка шаблона совпала до конца
                        return i;
        };
        
        return (-1); // no string found                
}

