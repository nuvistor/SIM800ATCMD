/*-------------------------------------------------------
at_parser.h
-------------------------------------------------------*/
#ifndef _AT_PARSER_H
#define _AT_PARSER_H

#include <stdint.h>

uint8_t parse_int( uint8_t* str, uint8_t field_num, uint8_t max_len, uint8_t div, int16_t* result );
uint8_t parse_long( uint8_t* str, uint8_t field_num, uint8_t max_len, uint8_t div, int32_t* result );
uint8_t parse_str_quoted( uint8_t *src_str, uint8_t field_num, uint8_t max_len, uint8_t delim, uint8_t *dst_str );
int16_t resp_cmp( uint8_t *str, flash uint8_t *resp_sample[], uint8_t items );

#endif _AT_PARSER_H