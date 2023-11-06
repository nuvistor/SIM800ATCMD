/*-------------------------------------------------------
sms_proc.c
SMS Processor
-------------------------------------------------------*/

#include "PT\pt.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sim800\sim800_at.h"
#include "sim800\at_parser.h"
#include "main.h"
#include "logger.h"
#include "ds1307.h"
#include "utils.h"
#include "ba_ict104.h"
#include "mdb_slave.h"

/*-------------------------------------------------------

-------------------------------------------------------*/
int16_t resp_cmp( uint8_t *str, flash uint8_t *resp_sample[], uint8_t items );

/*-------------------------------------------------------

-------------------------------------------------------*/
PT_THREAD(sms_read_thread(struct pt *pt));
PT_THREAD(sms_send_thread(struct pt *pt));
//
extern struct pt sms_read_pt;
extern struct pt sms_send_pt;
/*-------------------------------------------------------

-------------------------------------------------------*/
extern eeprom uint8_t apn_name[128];
extern eeprom uint8_t server_name[100];
//
extern uint8_t server_connected;
extern uint8_t need_server_connect;

static uint8_t sms_tmp_str[256];


/*-------------------------------------------------------
обработка входящих команд через SMS
-------------------------------------------------------*/ 

// SMS
struct pt sms_cmd_pt;

PT_THREAD(sms_cmd_thread(struct pt *pt))
{
    int16_t sms_cmd_code;
    uint16_t slaves_cnt;
    uint8_t i, j;
    uint8_t arg_err;
    
    //int16_t tmp_credit;
    int16_t tmpi16[4];
    int32_t tmpi32;
    //uint8_t *p8;
    //
    PT_BEGIN(pt);
    //
    
    // --------------------- если запрос на отправку смс        
                
    // ---------------------  читать SMS и расшифровать упр. команды
    if( modem_state.isSMSArrived )
    {
            modem_state.isSMSArrived = 0;
            // Read SMS
            PT_SPAWN( pt, &sms_read_pt, sms_read_thread(&sms_read_pt) ); // запустить поток чтения SMS
            //
            if( modem_state.isSMSReadSuccess )
            {
                    memset( sms_tmp_str,     0, sizeof(sms_tmp_str) ); // clear bufs
                    memset( sms.to_send_str, 0, sizeof(sms.to_send_str) ); 
                    //
					strcpyf( sms.to_send_str, "SMS TEXT TO SEND" );
					
					modem_state.isSMS_SendRq = TRUE;
					
					// --------------------- если запрос на отправку смс
                            if( modem_state.isSMS_SendRq )
                            { 
                                modem_state.isSMS_SendRq = FALSE;
                                PT_SPAWN( pt, &sms_send_pt, sms_send_thread(&sms_send_pt) ); // запустить поток отправки SMS
                            };
					
                  
                    //
            };        
    };
    
    //
    PT_END(pt);
}