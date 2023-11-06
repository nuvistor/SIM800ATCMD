/*-------------------------------------------------------
SIM800 UART IO
-------------------------------------------------------*/

#ifndef _UARTSIM800_H
#define _UARTSIM800_H
#include <stdint.h>

#define SIM800_RESET_ENABLE()   PORTB.7=1  
#define SIM800_RESET_DISABLE()  PORTB.7=0

//
void sim_uart_init( uint32_t baud );
void sim_uart_rx_handler( void );
void sim_uart_tx_handler( void );
//
void sim_dbg_rx( flash uint8_t* s );
// AT mode
void sim_set_at_mode( uint8_t mode );
uint8_t sim_get_at_mode( void );
//
uint8_t sim_check_error( void );
//
void sim_flush_rx_buf( void );
//
void sim_putchar( uint8_t c );
void sim_putsf( flash uint8_t *str );
void sim_putsee( eeprom uint8_t *str, uint8_t max_len );
void sim_puts( uint8_t *str );
void sim_put_int( int16_t v );
void sim_send_cmd( flash uint8_t *str, uint8_t tmr, uint16_t timeout );
//
uint8_t sim_get_line_cnt( void ); // число ответов в буфере
//uint8_t* sim_readline( void ); // читать ответ из буфера
//

/*-------------------------------------------------------

-------------------------------------------------------*/
//extern volatile uint8_t at_resp_error;
extern volatile uint8_t isDataPrompt;

extern uint8_t sim_rx_buf[];


#endif _UARTSIM800_H

/*-------------------------------------------------------

-------------------------------------------------------*/