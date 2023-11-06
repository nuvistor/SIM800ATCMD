/*-------------------------------------------------------
Server Processor
server_proc.h
-------------------------------------------------------*/
#ifndef _SERVER_PROC
#define _SERVER_PROC

typedef enum{
STA_SERVER_START_SEND,
STA_SERVER_START_WAIT,
STA_SERVER_CONNECTED
}server_conn_state_t;


//void server_conn_proc( void );
void server_proc( void );

#endif _SERVER_PROC