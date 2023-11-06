/*-------------------------------------------------------
sms_proc.h
SMS Processor
-------------------------------------------------------*/
#ifndef _SMS_PROC
#define _SMS_PROC

#include "PT\pt.h"

extern PT_THREAD(sms_cmd_thread(struct pt *pt));

extern struct pt sms_cmd_pt;

void sms_admin_numbers_map_load( void );

#endif _SMS_PROC