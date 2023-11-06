/*-------------------------------------------------------
Server Processor
server_proc.c
-------------------------------------------------------*/
#include "main.h"
#include "PT\pt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"
#include "sw_timers.h"
#include "sim800\sim800_at.h"
#include "sim800\at_parser.h"
#include "server_proc.h"
#include "utils.h"
#include "mdb_slave.h"

/*-------------------------------------------------------

-------------------------------------------------------*/
//extern tcp_conn_state_t tcp_conn_state; // ������� GPRS ���������� � ��������

extern uint8_t *resp; // ��������� ����� �� �� ������� ��� URC ���
extern uint8_t server_connected;

uint8_t srvr_tmp_str[256];




uint8_t need_server_connect=FALSE;

/*-------------------------------------------------------
���������� �����������
GPRS ����� ������� � ��������
-------------------------------------------------------*/
#define SERVER_MAX_RESP_TIME        (30*10)


/*-------------------------------------------------------

-------------------------------------------------------*/
void server_reset( void )
{
        device_reset();
        //.... 
        modem_state.isTCPTxRq = TRUE;
}

/*-------------------------------------------------------

-------------------------------------------------------*/
struct pt server_cmd_pt;

PT_THREAD(server_cmd_thread(struct pt *pt))
{
        uint16_t tmpi16;
        static int16_t cmd_code = -1;
        //
        PT_BEGIN(pt);
        
        
        //
        while(1)
        {
                // --- �������� ������� ������ �� ������� ����� ���� ���������� �����������
                if( sw_get_timeout( TIM1_SERVER_RESP ) == 0 )
                {
                    // ���� ������ ����� �� ��������
                    // ������������� ����������(�������, ����� ����� �������)
                    modem_state.TCPState = TCP_ERROR;
                    PT_EXIT(pt);
                    // 
                };
                //
                
                //
                if( modem_state.isTCPRxAccepted ) // ����� �������?
                {
		
						//
						//....
                        //
                        modem_state.isTCPRxAccepted = FALSE;  
                }else
                {
                        PT_YIELD(pt);
                };
        };
                
        //
        PT_END(pt);

}

/*-------------------------------------------------------

-------------------------------------------------------*/
void server_proc(void)
{   
    switch( modem_state.TCPState ) // ��������� TCP GPRS PDP
    { 
            case TCP_CONNECTED:
                    server_cmd_thread(&server_cmd_pt);                 
                    break;
            case TCP_INIT:
                    PT_INIT(&server_cmd_pt); 
                    break;
            case TCP_CLOSED:
                    server_connected=FALSE;
                    break;
            case TCP_ERROR:
                    server_connected=FALSE;
                    break;
    };
}