
/*-------------------------------------------------------
SIM800 UART IO
-------------------------------------------------------*/
#include <mega2560.h>
#include "sim800\uart_sim800_io.h"
#include <stdint.h>
#include "main.h"
#include <delay.h>

#include <string.h>
#include <stdlib.h> 
#include "sw_timers.h"
#include "queue.h"


#ifndef RXB8
#define RXB8 1
#endif

#ifndef TXB8
#define TXB8 0
#endif

#ifndef UPE
#define UPE 2
#endif

#ifndef DOR
#define DOR 3
#endif

#ifndef FE
#define FE 4
#endif

#ifndef UDRE
#define UDRE 5
#endif

#ifndef RXC
#define RXC 7
#endif

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)


// Buffer size
#define MODEM_RX_BUF_SZ 256
#define MODEM_TX_BUF_SZ 256

uint8_t sim_rx_buf[ MODEM_RX_BUF_SZ ]; // байтовый поток приёма(циклический буфер)
queue_t sim_rx_stream;

uint8_t sim_tx_buf[ MODEM_TX_BUF_SZ ]; // байтовый поток передачи
queue_t sim_tx_stream;

volatile uint8_t sim_rx_cnt = 0;


uint8_t at_mode = TRUE; // at cmd mode or binary data mode
//volatile uint8_t at_resp_complete = FALSE; // флаг получения ответа на АТ команду, URC, CME или иного сообщения 
//uint8_t at_resp_rx_sta = 0; // статус приёма ответа
volatile uint8_t at_resp_error = FALSE; // AT response sync error
volatile uint8_t isDataPrompt = FALSE;
uint8_t at_resp_byte_cnt = 0; // счётчик байтов длины ответа
//uint8_t resp_first_byte = 0;

//static uint8_t resp_str[ MODEM_RX_BUF_SZ ]; // строка ответа, извлекаемая из буфера FIFO

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_put_line_fifo( uint8_t ptr, uint8_t len );

/*-------------------------------------------------------
Init UART
-------------------------------------------------------*/
void sim_uart_init( uint32_t baud )
{
        buf_init( &sim_tx_stream, sim_tx_buf, MODEM_TX_BUF_SZ ); // инит потоков
        buf_init( &sim_rx_stream, sim_rx_buf, MODEM_RX_BUF_SZ );
        
        // UART pins cfg
        // PJ0=RX
        // PJ1=TX
        CLR_BIT(DDRJ,0); // input/RXD
        //CLR_BIT(PORTJ,0);// no pull-up
        SET_BIT(PORTJ,0);// pull up
        //
        SET_BIT(DDRJ,1); // output/TXD
        CLR_BIT(PORTJ,1);// =1
        //
        
        switch (baud)
        { 
                case 9600:
                        // USART1 initialization
                        // Communication Parameters: 8 Data, 1 Stop, No Parity
                        // USART1 Receiver: On
                        // USART1 Transmitter: On
                        // USART1 Mode: Asynchronous
                        // USART1 Baud Rate: 9600
                        UCSR3A=0x00;
                        UCSR3B=0xD8;
                        UCSR3C=0x06;
                        UBRR3H=0x00;
                        UBRR3L=0x67;
                        break;
                case 115200:
                        // USART1 initialization
                        // Communication Parameters: 8 Data, 1 Stop, No Parity
                        // USART1 Receiver: On
                        // USART1 Transmitter: On
                        // USART1 Mode: Asynchronous
                        // USART1 Baud Rate: 57600 (Double Speed Mode) = 115200
                        UCSR3A=0x02;
                        UCSR3B=0xD8;
                        UCSR3C=0x06;
                        UBRR3H=0x00;
                        UBRR3L=0x22;
                        break;
        };
}

           
/*-------------------------------------------------------
simulate RX data
-------------------------------------------------------*/
void sim_dbg_rx( flash uint8_t* s )
{
    while( *s )
            { 
                    buf_put_data( &sim_rx_stream, *s++ );
                    //sim_putchar( *str++ );
            };
}

/*-------------------------------------------------------
RX IRQ handler
-------------------------------------------------------*/
#define RESPONSE_LINE_TIMEOUT_VAL    (1*10) // строка ответа должна завершиться за 1 сек
//uint8_t sim_rx_line_complete_timeout = 0; // таймаут на завершение приёма команды

void sim_uart_rx_handler( void )
{
    char status, data;
    status = UCSR3A;
    data = UDR3;
    
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
    {  
        buf_put_data( &sim_rx_stream, data );  
    }
}


/*-------------------------------------------------------
TX IRQ handler
-------------------------------------------------------*/
void sim_uart_tx_handler( void )
{
        //if( buf_get_cnt( &sim_tx_stream ) > 0 )
        if( sim_tx_stream.cnt > 0 )
        {
                UDR3 = buf_get_data_unsafe( &sim_tx_stream ); 
        };       
}     


/*-------------------------------------------------------
AT or data mode
-------------------------------------------------------*/
void sim_set_at_mode( uint8_t mode )
{
        at_mode = mode;
}

/*-------------------------------------------------------

-------------------------------------------------------*/
uint8_t sim_get_at_mode( void )
{
        return at_mode;
}


/*-------------------------------------------------------
check response error 
-------------------------------------------------------*/
uint8_t sim_check_error( void )
{
        return at_resp_error;
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_flush_rx_buf( void )
{
        CLI();
        sim_rx_cnt = 0;
        at_resp_byte_cnt = 0;
        //at_resp_rx_sta = 0;
        at_resp_error = FALSE;
        isDataPrompt = 0;
        //sim_rx_line_complete_timeout = 0;
        //at_resp_complete = FALSE;
        SEI();
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_putchar( uint8_t c )
{
    while( buf_get_cnt( &sim_tx_stream ) == MODEM_TX_BUF_SZ );
    CLI();
    //if( (buf_get_cnt( &sim_tx_stream ) >0) || ((UCSR3A & DATA_REGISTER_EMPTY)==0)  )
    if( (sim_tx_stream.cnt>0) || ((UCSR3A & DATA_REGISTER_EMPTY)==0)  )
    {
        buf_put_data( &sim_tx_stream, c );
    }else
    {
        UDR3=c;
    };
    SEI();
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_putsf( flash uint8_t *str )
{
        while( *str )
        { 
                sim_putchar( *str++ );
        };
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_putsee( eeprom uint8_t *str, uint8_t max_len )
{
        uint8_t i=0;
        
        while( (*str) && (i<max_len) )
        { 
                sim_putchar( *str++ );
                i++;
        };
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_puts( uint8_t *str )
{
        while( *str )
        { 
                sim_putchar( *str++ );
        };
}

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim_put_int( int16_t v )
{
        uint8_t str[10];
        
        memset( str, 0, 10 );
        
        itoa( v, str );
        
        sim_puts( str );
}

/*-------------------------------------------------------
Args: cmd, timeout*10
-------------------------------------------------------*/
void sim_send_cmd( flash uint8_t *str, uint8_t tmr, uint16_t timeout )
{
        sim_putsf( str );
        sw_set_timeout( tmr, timeout );
}










