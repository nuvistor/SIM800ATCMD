/*-------------------------------------------------------
sim800_at.c
AT commands and state processor
-------------------------------------------------------*/
#include "main.h"
#include "sim800\uart_sim800_io.h"
#include "PT\pt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"
#include "sw_timers.h"
#include "utils.h"
#include "sim800\at_parser.h"
#include "queue.h"
#include "ds1307.h"
#include "sim800_at.h"
#include "sms_proc.h"
#include "server_proc.h"


/*-------------------------------------------------------

-------------------------------------------------------*/


uint8_t server_connected = FALSE;

uint8_t resp_str[ 256 ]; // строка ответа, извлекаемая из буфера FIFO
uint8_t resp_byte_cnt = 0;
uint8_t resp_str_state = 0;
uint8_t resp_len = 0;
uint8_t resp_str_capture = FALSE;

eeprom uint8_t ee_isServerEnable=1;

/*-------------------------------------------------------

-------------------------------------------------------*/
void sim800_at_init( void );

/*-------------------------------------------------------

-------------------------------------------------------*/
flash uint8_t *RespStrCodes[]={
// Generic response
"OK", // When TA returns to Command mode after call release
"ERROR", //
// URC
// error codes
"+CME ERROR: ",
"+CMS ERROR: ",
// normal state urc
//"+CCWA:", // Indication of a call that is currently waiting and can be accepted.
"+CLIP:", // The calling line identity (CLI) of the calling party when receiving a mobile
"+CRING:", // Indicates incoming call to the TE if extended format is enabled
"+CREG:", // There is a change in the MT network registration status or a change of the network cell
"+CCWV", // Shortly before the ACM (Accumulated Call Meter) maximum value is reached.

"+CMTI:", // Indicates that new message has been received
"+CMT:", // Indicates that new message has been received
"+CBM:", // Indicates that new cell broadcast message has been received
"+CDS:", // Indicates that new SMS status report has been received

"+COLP:", // The presentation of the COL (Connected Line) at the TE for a mobile originated call
"+CSSU:", // Presentation status during a mobile terminated call setup or during a call, or when a forward check supplementary service notification is received
"+CSSI:", // Presentation status after a mobile originated call setup
"+CLCC:", // Report a list of current calls of ME automatically when the current call status changes

"*PSNWID:", // Refresh network name by network.
"*PSUTTZ:", // Refresh time and time zone by network
"+CTZV:", // Refresh network time zone by network
"DST:", // Refresh Network Daylight Saving Time by network

"+CSMINS:", // Indicates whether SIM card has been inserted
"CDRIND:", // Indicates whether a CS voice call, CS data has been terminated
"+CHF:", // Indicates the current channel
"+CENG", // Report of network information

"MO RING", // Shows call state of mobile originated call: the call is alerted 
"MO CONNECTED", // Shows call state of mobile originated call: the call is established.
"+CPIN:", // Indicates whether some password is required or not
"+CSQN:", // Displays signal strength and channel bit error rate when <rssi>, <ber>values change
//"+SIMTONE:", // The generated tone playing is stopped or completed
//"+STTONE:", // The SIM Toolkit tone playing is stopped or completed
"+CR:", /* An intermediate result code is transmitted during connect negotiation
when the TA has determined the speed  and quality of service to be used, before
any error control or data compression reports are transmitted, and before any
final result code (e.g. CONNECT)  appears*/
"+CUSD:", // Indicates an USSD response from the  network, or network initiated operation
"RING", // An incoming call signal from network is  detected
/*/"NORMAL POWER DOWN", // SIM800 is powered down by the PWRKEY pin or AT command “AT+CPOWD=1”
//"UNDER-VOLTAGE POWER DOWN", // Under-voltage automatic power down
//"UNDER-VOLTAGE WARNNING", // under-voltage warning
//"OVER-VOLTAGE POWER DOWN", //
//"OVER-VOLTAGE WARNNING", // */
//"CHARGE-ONLY MODE", // The module is charging by charger(require hardware support)
"RDY", /* Power on procedure is completed, and  the module is ready to operate at fixed
baud rate. (This URC does not appear when auto-bauding function is active).*/
"Call Ready", // Module is powered on and phonebook initialization procedure is over.
"SMS Ready", // Module is powered on and SMS initialization procedure is over.
"+CFUN:", // Phone functionality indication (This URC does not appear when auto-bauding function is active)
"CONNECT OK", // TCP/ UDP connection is successful
"CONNECT FAIL", // TCP/UDP connection fails
"ALREADY CONNECT", // TCP/UDP connection exists
"CONNECT", // TCP/UDP connection in channel mode is successful/Response in case of data call, if successfully connected
"SEND OK", // Data sending is successfu
"CLOSED", // TCP/UDP connection is closed
"RECV FROM:", // shows remote IP address and port (only in single connection mode
"+IPD", // display transfer protocol in IP header to received data or not (only in single connection mode)
"+RECEIVE", // Received data from remote client (only in multiple connection mode)
"REMOTE IP:", // Remote client connected in
"+CDNSGIP:", // DNS SUCCESS/FAIL
"+PDP: DEACT", // GPRS is disconnected by network
"+SAPBR", // The bearer based on IP connection of SIMCom application is deactivated
/*/ httpaction
//"+FTPGET:", // FTPGET session
//"+FTPPUT:", // It is ready to upload data / FTP return result 
//"+FTPDELE:", // FTP delete session
//"+FTPSIZE:", // FTP size session
//"+FTPMKD:", // FTP create directory (not supported for all versions)
//"+FTPRMD:", // FTP delete directory (not supported for all versions)
//"+FTPLIST:", // FTP list session (not supported for all versions) */
"+CGREG:", // Network Registration Status
//"ALARM RING", // Indicate expired alarm.
//"+CALV:", // // Indicate expired alarm.
"NO CARRIER", // Response if no connection
"NO DIALTONE", // If no dial tone and (parameter setting ATX2 or ATX4)
"BUSY", // If busy and (parameter setting ATX3 or ATX4)
"NO ANSWER", // If the remote station does not answer
"+GCAP:", // TA reports a list of additional capabilities
//"Revision:", // TA reports one or more lines of information text which permit the user to identify the revision of software release
"+ICF:", // This parameter setting determines the serial interface character framing format and parity received by TA from TE
"+IFC:", // This parameter setting determines the data flow control on the serial interface for data mode
"+IPR:", // This parameter setting determines the data rate of the TA on the serial interface.
"+CBST:", // TA selects the bearer service <name> with data rate <speed>, and the connection element <ce> to be used when data calls are originated
//"+CCFC:", // TA controls the call forwarding supplementary service. Registration, erasure, activation, deactivation, and status query are supported
"+CEER:", // TA returns an extended report of the reason for the last call release
"+CSCS:", // Sets which character set <chset> are used by the TE. The TA can then convert character strings correctly between the TE and ME character sets
"+CSTA:", // Type of address octet in integer format;
//"+CHLD:", // Call Hold and Multiparty
//"+CLCC:", // List Current Calls of ME
//"+CLCK:", // Facility Lock. This Command is used to lock, unlock or interrogate a ME or a network facility <fac>. Password is normally needed to do such actions. 
//"+CLIR:", // TA restricts or enables the presentation of the CLI to the called party when originating a call
"+CMEE:", // Report Mobile Equipment Error. TA disables or enables the use of result code +CME ERROR: <err> as an indication of an error relating to the functionality of the ME
//"+COLP:", // Connected Line Identification Presentation. TA enables or disables the presentation of the COL (Connected Line) at the TE for a mobile originated call. 
"+COPS:", // Operator Selection. TA forces an attempt to select and register the GSM network operator. If the selected operator is not available, no other operator shall be selected
"+CPAS:", // Phone Activity Status.
"+CRC:", // TA controls whether or not the extended format of incoming call indication is used
"+CRLP:", // TA sets radio link protocol (RLP) parameters used when non-transparent data calls are setup
"+CRSM: ", // Restricted SIM Access
"+CSQ:", // Signal Quality Report
"+CMUX:", // Multiplexer Control
"+CNUM:", // Subscriber Number
"+CCLK:", // Clock. String type(string should be included in quotation marks) value; format is "yy/MM/dd,hh:mm:ss±zz" 
// AT Commands According to 3GPP TS 27.005
"+CMGF:", // Select SMS Message Format
"+CMGL:", // List SMS Messages from Preferred Store
"+CMGR:", // Read SMS Message
"+CMGS:", // Send SMS Message
"+CMGW:", // Write SMS Message to Memory
"+CMSS:", // Send SMS Message from Storage
//"+CPMS:", // Preferred SMS Message Storage
//"+CSMS:", // Select Message Service
//5 AT Commands for SIM Application Toolkit
//6 AT Commands Special for SIMCom
"+CCALR:", // Call Ready Query.ME returns the status of result code presentation and an integer <n>which shows whether the module is currently ready for phone call
//7 AT Commands for GPRS Support
"+CGATT:", // <<<< AT+CGATT Attach or Detach from GPRS Service
"+CGPADDR:", // Show PDP Address
"+CGEV:", // AT+CGEREP Control Unsolicited GPRS Event Reporting
// 8 AT Commands for TCPIP Application Toolkit
"DATA ACCEPT:", //AT+CIPSEND Send Data Through TCP or UDP Connection 
"SEND FAIL", // 
"+CIPACK:", // Query Previous Connection Data Transmitting State 
"CLOSE OK", // AT+CIPCLOSE Close TCP or UDP Connection
"CLOSE", // When +CIPQSEND=0 and the remote server no response, after 645 seconds, “CLOSE” will be reported 
"SHUT OK", // AT+CIPSHUT Deactivate GPRS PDP Context 
"+CIPSTATUS:", // AT+CIPSTATUS Query Current Connection Status 
"STATE:", // AT+CIPSTATUS Query Current Connection Status
//"+CDNSGIP:", // AT+CDNSGIP Query the IP Address of Given Domain Name 
//"+RECV FROM:", // 
"+CIPRXGET:", // AT+CIPRXGET Get Data from Network Manually
// 9 AT Commands for IP Application
//10 AT Commands for PING Support
"+CIPPING:", // AT+CIPPING PING Request
//11 AT Commands for HTTP Application 
"DOWNLOAD", // AT+HTTPDATA Input HTTP Data
"+HTTPACTION:", // URC Indicates HTTP method, Status Code responded by remote server and the length of data got 
"+HTTPREAD:", // Read the HTTP Server Response 
"+HTTPSTATUS:", // AT+HTTPSTATUS Read HTTP Status 
"+HTTPHEAD:", // AT+HTTPHEAD Read the HTTP Header Information of Server Response 
};

typedef enum{
// Generic response
RSP_NOTHING=(int16_t)(-1),
//
RSP_OK=0,
RSP_ERROR,
// URC
RSP_CME_ERR,
RSP_CMS_ERR,
//
//RSP_CCWA,

RSP_CLIP,
RSP_CRING,
RSP_CREG,
RSP_CCWV,

RSP_CMTI,
RSP_CMT,
RSP_CBM,
RSP_CDS,

RSP_COLP,
RSP_CSSU,
RSP_CSSI,
RSP_CLCC,

RSP_PSNWID,
RSP_PSUTTZ,
RSP_CTZV,
RSP_DST,

RSP_CSMINS,
RSP_CDRIND,
RSP_CHF,
RSP_CENG,

RSP_MO_RING,
RSP_MO_CONNECTED,
RSP_CPIN,
RSP_CSQN,

RSP_CR,
RSP_CUSD,
RSP_RING,
//
RSP_RDY,
RSP_CALL_READY,
RSP_SMS_READY,
//
RSP_CFUN,

RSP_CONNECT_OK,
RSP_CONNECT_FAIL,
RSP_ALREADY_CONNECT,
RSP_CONNECT,
RSP_SEND_OK,
RSP_CLOSED,
RSP_RECV_FROM,
RSP_IPD,
RSP_RECEIVE,
RSP_REMOTE_IP,
RSP_CDNSGIP,
RSP_PDP_DEACT,
RSP_SAPBR,
RSP_CGREG,
RSP_NO_CARRIER,
RSP_NO_DIALTONE,
RSP_BUSY,
RSP_NO_ANSWER,
RSP_GCAP,
RSP_ICF,
RSP_IFC,
RSP_IPR,
RSP_CBST,
RSP_CEER,
RSP_CSCS,
RSP_CSTA,
//RSP_CLCC,
RSP_CMEE,
//RSP_COLP,
RSP_COPS,
RSP_CPAS,
RSP_CRC,
RSP_CRLP,
RSP_CRSM,
RSP_CSQ,
RSP_CMUX,
RSP_CNUM,
RSP_CCLK,
// AT Commands According to 3GPP TS 27.005
RSP_CMGF,
RSP_CMGL,
RSP_CMGR,
RSP_CMGS,
RSP_CMGW,
RSP_CMSS,
// AT Commands Special for SIMCom
RSP_CCALR,
// AT Commands for GPRS Support
RSP_CGATT, // <<<<<<<<<<
RSP_CGPADDR,
RSP_CGEV,
// 8 AT Commands for TCPIP Application Toolkit
RSP_DATA_ACCEPT,
RSP_SEND_FAIL,
RSP_CIPACK,
RSP_CLOSE_OK,
RSP_CLOSE,
RSP_SHUT_OK,
RSP_CIPSTATUS,
RSP_STATE,
RSP_CIPRXGET,
RSP_CIPPING,
RSP_DOWNLOAD,

RSP_TOTAL_ITEMS,
} resp_code_t;

//resp_code_t 
int16_t resp_code = RSP_NOTHING;



/*-------------------------------------------------------

-------------------------------------------------------*/
uint8_t *resp; // текстовый ответ на АТ команду или URC код
//uint8_t resp_cnt=0; // сколько ответов в буфере

modem_state_t modem_state;

sms_t sms;

          
/*-------------------------------------------------------
потоки команд начальной инициализации модема
-------------------------------------------------------*/
static struct pt child_init_pt;

/*-------------------------------------------------------
// Echo OFF ATE0
-------------------------------------------------------*/
uint8_t ate0_tmr;
static PT_THREAD(EchoOff_thread(struct pt *pt, uint8_t tmr)) // ATE0
{
        PT_BEGIN(pt);
        
        ate0_tmr = tmr;
        sim_send_cmd("ATE0\r", tmr, 1*10); // установить связь с модемом/синхронизация интерфейса
        PT_WAIT_WHILE( pt, (sw_get_timeout(tmr)>0)&&(resp_code!=RSP_OK) ); // событие приёма
        if( sw_get_timeout(tmr)==0 )
        {
                PT_EXIT(pt); // error
        };        
        
        if( resp_code == RSP_OK )
        {
                modem_state.isSynch = 1;
                resp_code = RSP_NOTHING; // supress response         
        };
        
        PT_END(pt);
}

/*-------------------------------------------------------
AT+CFUN Set Phone Functionality
-- Reset
-------------------------------------------------------*/
static PT_THREAD(CFUN_thread(struct pt *pt)) // ATE0
{
        PT_BEGIN(pt);

        sim_send_cmd( "AT+CFUN=1,1\r", TIM3_SIM_CMD,  10*10 ); // reset 
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) );
        
        if(resp_code==RSP_OK)
        { 
                resp_code = RSP_NOTHING;
        };
        
        PT_END(pt);
}

/*-------------------------------------------------------
AT+CPIN Enter PIN
-------------------------------------------------------*/
static PT_THREAD(CPIN_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        
        sim_send_cmd("AT+CPIN?\r", TIM3_SIM_CMD, 5*10);
        
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CPIN)&&
                                (resp_code!=RSP_CME_ERR)&&
                                (sw_get_timeout(TIM3_SIM_CMD)>0) ); // +CPIN
        if( resp_code == RSP_CPIN )
        { 
                resp_code = RSP_NOTHING;
                if( strcmpf( resp, "+CPIN: READY" ) == 0 )
                { // SIM READY
                        modem_state.isPinReady = TRUE;
                }else // введите пин-код
                {
                };
                PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK        
        };                                                                                       
        
        resp_code = RSP_NOTHING;
        
        PT_END(pt);
}

/*-------------------------------------------------------
AT+CREG Network Registration
-------------------------------------------------------*/
static PT_THREAD(CREG_thread(struct pt *pt))
{
        uint8_t tmpu8;
        
        PT_BEGIN(pt);
        
        sim_send_cmd("AT+CREG?\r", TIM3_SIM_CMD,  2*10);
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CREG)&&(resp_code!=RSP_CME_ERR)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // +CREG: <n>,<sta> 
        
        if( resp_code == RSP_CREG )
        { 
                resp_code = RSP_NOTHING;
                tmpu8 = resp[9]; // <sta>
                modem_state.CREGSatus = tmpu8;
                if( (tmpu8 == '0') ||   // Not registered, MT is not currently searching a new operator to register to
                        (tmpu8=='3') || // Registration denied
                        (tmpu8=='4') )  // Unknown
                {
                        //PT_RESTART( pt );
                }else if( (tmpu8=='1') || // Registered, home network
                        (tmpu8=='5') )    // Registered, roaming
                {
                        modem_state.isRegistered = TRUE;
                        // OK
                }else if( (tmpu8=='2') ) // Not registered, but MT is currently searching a new operator to register to
                {
                        // ждать регистрацию в сети
                }else // data error
                { 
                        //PT_RESTART( pt );
                };
                
                PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK               
        };
        
        resp_code = RSP_NOTHING;
        
        PT_END(pt);
};

/*-------------------------------------------------------
AT+CCALR Call Ready Query
-------------------------------------------------------*/
static PT_THREAD(CCALR_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        
        sim_send_cmd( "AT+CCALR?\r", TIM3_SIM_CMD,  2*10 ); 
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CCALR)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
        if( resp_code == RSP_CCALR )
            modem_state.isCallReady = 1; 
                            
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
        resp_code = RSP_NOTHING;        
        
        PT_END(pt);
}

/*-------------------------------------------------------
Set fixed Baudrate
-------------------------------------------------------*/
static PT_THREAD(IPR_thread(struct pt *pt))
{
        int32_t ipr;
        
        PT_BEGIN(pt);
        //
        sim_send_cmd( "AT+IPR?\r", TIM3_SIM_CMD,  2*10 ); 
        PT_WAIT_WHILE( pt, (resp_code!=RSP_IPR)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
        if( resp_code == RSP_IPR )
        {
            if( parse_long(resp, 1, 6, ':', &ipr) )
            //if( sscanf( resp, "+IPR:%lu", ipr) != -1 )
            { 
                if( ipr != 57600 )
                { // set new IPR baudrate
                    resp_code = RSP_NOTHING;
                    PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
                    //
                    resp_code = RSP_NOTHING;
                    sim_send_cmd( "AT+IPR=57600\r", TIM3_SIM_CMD,  2*10 ); 
                    PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
                    resp_code = RSP_NOTHING;
                    //
                    sim_send_cmd( "AT&W\r", TIM3_SIM_CMD,  2*10 ); // Save settings
                    PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
                }else
                {
                        resp_code = RSP_NOTHING;
                        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
                };
            }else
            { 
                resp_code = RSP_NOTHING;
                PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
            };
        }; 
        //
        resp_code = RSP_NOTHING;        
        //
        PT_END(pt);
}



/*-------------------------------------------------------
поток начальной инициализации модема
-------------------------------------------------------*/
static struct pt sim_init_pt;

static uint8_t i8_init;
static PT_THREAD(sim_init_thread(struct pt *pt))
{       
        PT_BEGIN(pt);
        //
        memset( &modem_state, 0, sizeof(modem_state) ); // сбросить все флажки
        memset( &sms, 0, sizeof( sms ) ); // очистить строки
                   
        //------------- SIM reset           
        SIM800_RESET_ENABLE();
        gsm_pwr_on( 0 );
        sw_set_timeout( TIM3_SIM_CMD, 5 ); // 300ms
        PT_WAIT_WHILE( pt, sw_get_timeout(TIM3_SIM_CMD)>0 );
        //------------- SIM Boot
        SIM800_RESET_DISABLE();
        gsm_pwr_on( 1 );
        sw_set_timeout( TIM3_SIM_CMD, 10*10 ); // 3sec
        //------------- Synch
        sim_set_at_mode( 1 ); // AT cmd mode
        //at_resp_error = FALSE;
        sim_flush_rx_buf();
        resp_str_capture = FALSE;
        resp_len = resp_byte_cnt;
        resp_byte_cnt = 0;
        resp_code = RSP_NOTHING;
        PT_WAIT_WHILE( pt, (sw_get_timeout(TIM3_SIM_CMD)>0)&&(resp_code!=RSP_CFUN) ); // задержка на вкл GSM модуля 
        
        //------------- Synch
        sim_set_at_mode( 1 ); // AT cmd mode
        //at_resp_error = FALSE;
        sim_flush_rx_buf();
        resp_str_capture = FALSE;
        resp_len = resp_byte_cnt;
        resp_byte_cnt = 0;
        //
        PT_SPAWN( pt, &child_init_pt, EchoOff_thread(&child_init_pt,TIM3_SIM_CMD) ); // ATE0
        //
        PT_SPAWN( pt, &child_init_pt, IPR_thread(&child_init_pt) );     // Set Baudrate
        //
        modem_state.isPinReady = 0;
        modem_state.isCallReady = 0;
        modem_state.isSMSReady = 0;
        modem_state.isCMEErr = 0;
        modem_state.isCMSErr = 0;
        modem_state.isRegistered = 0;
        modem_state.isSynch = 0;
        modem_state.isModemInit = 0;
        PT_SPAWN( pt, &child_init_pt, CFUN_thread(&child_init_pt) );    // Reset Module
        sw_set_timeout( TIM3_SIM_CMD, 5*10 ); // 3sec
        PT_WAIT_WHILE( pt, (sw_get_timeout(TIM3_SIM_CMD)>0)&&(resp_code!=RSP_CFUN) ); // задержка на вкл GSM модуля
        //------------- Synch
        sim_set_at_mode( 1 ); // AT cmd mode
        sim_flush_rx_buf();
        resp_str_capture = FALSE;
        resp_len = resp_byte_cnt;
        resp_byte_cnt = 0;
        //
        //      
        resp_code=RSP_NOTHING;        
        
        // Echo OFF ATE0
        modem_state.isSynch = 0;
        for( i8_init=0; (i8_init<5) && (!modem_state.isSynch); i8_init++ )
        {
            PT_SPAWN( pt, &child_init_pt, EchoOff_thread(&child_init_pt,TIM3_SIM_CMD) );
            /*if( modem_state.isSynch )
            { 
                    NOP();
                    break;;
            };*/
        };
        if( !modem_state.isSynch )
        { 
                    NOP();
                    PT_RESTART( pt );
        };
                      
       //-------------- ПИН код для SIM
       for(i8_init=0; (i8_init<10) && (!modem_state.isPinReady); i8_init++)
       { 
           PT_SPAWN( pt, &child_init_pt, CPIN_thread( &child_init_pt ) );
           //
           sw_set_timeout( TIM3_SIM_CMD, 1*10 );
           PT_WAIT_WHILE( pt, (sw_get_timeout(TIM3_SIM_CMD)>0) ); // задержка перед повторной отправкой запроса
           //
           ;
           /*if( modem_state.isPinReady )
           { 
                    NOP();
                    break;
           };*/
       };
       if( !modem_state.isPinReady )
       { 
                    NOP();
                    PT_RESTART( pt );
       }; 
        
        //-------------- Проверка и ожидание регистрации в сети
        i8_init = 0;
        do{                                 
                PT_SPAWN( pt, &child_init_pt, CREG_thread(&child_init_pt) );
                if( (modem_state.CREGSatus == '0') || // Not registered, MT is not currently searching a new operator to register to
                        (modem_state.CREGSatus == '3') ) // Registration denied
                { 
                        NOP();
                        PT_RESTART( pt );
                }else if( modem_state.CREGSatus == '2' ) // Not registered, but MT is currently searching a new operator to register to
                {
                        //sw_set_timeout( TIM3_SIM_CMD, 1*10 );
                        //PT_WAIT_WHILE( pt, (sw_get_timeout(TIM3_SIM_CMD)>0) ); // задержка перед повторной отправкой запроса
                };
                //
                if(!modem_state.isRegistered)
                {
                    sw_set_timeout( TIM3_SIM_CMD, 1*10 );
                    PT_WAIT_WHILE( pt, (sw_get_timeout(TIM3_SIM_CMD)>0) ); // задержка перед повторной отправкой запроса
                };
                //
                i8_init++;
                if( i8_init > 60 )
                {
                        NOP();
                        PT_RESTART( pt );        
                };  
        }while( !modem_state.isRegistered );
        
        // Call Ready Query --- готов звонить?
        while(!modem_state.isCallReady){ 
             PT_SPAWN( pt, &child_init_pt, CCALR_thread(&child_init_pt) );  // AT+CCALR
        };
        
        //-------------- set text mode
        sim_send_cmd( "AT+CMGF=1\r", TIM3_SIM_CMD,  2*10 ); // text mode
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
        resp_code = RSP_NOTHING;
        
        //-------------- set IRA text charset for TE
        sim_send_cmd( "AT+CSCS=\"IRA\"\r", TIM3_SIM_CMD,  1*10 ); // text mode IRA
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(resp_code!=RSP_CME_ERR)&&(sw_get_timeout(TIM3_SIM_CMD)>0) ); // OK
        resp_code = RSP_NOTHING;
        
        //-------------- Wait SMS ready
        sw_set_timeout( TIM3_SIM_CMD, 30*10 );
        PT_WAIT_WHILE( pt, (modem_state.isSMSReady == FALSE) && (sw_get_timeout(TIM3_SIM_CMD)>0) );
        if( sw_get_timeout(TIM3_SIM_CMD) == 0 )
        { 
                //PT_RESTART( pt ); // error timeout
                // *** смс готов по таймауту потому что в некоторых случаях(версии модуля или симки) не получаем URC SMS Ready ***
        };
               
        //-------------- Delete All SMS
        sim_send_cmd("AT+CMGDA=\"DEL ALL\"\r", TIM3_SIM_CMD,  25*10);
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(resp_code!=RSP_ERROR)&&(sw_get_timeout(TIM3_SIM_CMD)>0) );
        if( (sw_get_timeout(TIM3_SIM_CMD) == 0) || (resp_code==RSP_ERROR) )
        { 
                resp_code=RSP_NOTHING;
                PT_RESTART( pt ); // error or timeout
        };
        // response OK, ERROR, +CMS ERROR:
        if( resp_code == RSP_OK )
        {        
                resp_code=RSP_NOTHING;
                //break; // стирание закончено
        };
        //-------------- 
        modem_state.isModemInit = TRUE;
        
        
        PT_END(pt);
}

/*-------------------------------------------------------
Read and process sms
-------------------------------------------------------*/
struct pt sms_read_pt;


static struct pt child_sms_pt;
static struct pt child_tcp_pt;

uint8_t at_tmr;

static PT_THREAD(AT_Test_thread(struct pt *pt, uint8_t tmr))
{
        PT_BEGIN(pt);
        
        at_tmr = tmr;
        
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        
        modem_state.isATResponse = 0;
        modem_state.isOK = 0;
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT\r", at_tmr,  2*10 );
        PT_WAIT_WHILE( pt, (modem_state.isOK==0)&&(sw_get_timeout(at_tmr)>0) );
        if( modem_state.isOK )
        { 
                resp_code = RSP_NOTHING;
                modem_state.isATResponse = 1;
        };
        modem_state.isCmdWaitResp = 0;
        
        PT_END(pt);
}

PT_THREAD(sms_read_thread(struct pt *pt))
{        
        PT_BEGIN(pt);
        //
        PT_WAIT_WHILE( pt, modem_state.isSMSTxBusy || modem_state.isCmdWaitResp );        
        //
        //modem_state.isSMSRxBusy = 1; // lock SMS chanel
        modem_state.isSMSReadSuccess = 0;
        
        //------------- AT test
        PT_SPAWN( pt, &child_sms_pt, AT_Test_thread(&child_sms_pt, TIM6_SMS) );
        if( modem_state.isATResponse == 0 ) // error ?
        {
                //modem_state.isSMSRxBusy = 0;
                PT_EXIT( pt );
        };  
        //------------- 
        memset( sms.sender_text, 0, sizeof( sms.sender_text ) );
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CMGR=", TIM6_SMS,  6*10 );
        sim_put_int( modem_state.SMS_Idx );
        sim_putsf( "\r" );
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CMGR) && (resp_code!=RSP_ERROR) && (resp_code!=RSP_CME_ERR) && (sw_get_timeout(TIM6_SMS)>0) );
        
        if( resp_code == RSP_CMGR )
        {
                resp_code = RSP_NOTHING;
                resp_str_capture = 0; // supress +CMGR
                //resp_cnt = 0;
                // response:
                // +CMGR: "REC UNREAD","+number","","date_time"
                if( parse_str_quoted( resp, 1, 20-1, ',', sms.sender_number ) ) // пропустить 1 поле(со второго), макс длина строки номера=19 байт, 
                { 
                        //resp_code = RSP_NOTHING;
                        //-------------  SMS data
                        PT_WAIT_WHILE( pt, (!(resp_str_capture &&(resp_code==RSP_NOTHING)))&&(sw_get_timeout(TIM6_SMS)>0) ); // sms data
                        if( sw_get_timeout(TIM6_SMS) == 0 ) // synch err?
                        { 
                                //modem_state.isSMSRxBusy = 0; // unlock SMS chanel
                                modem_state.isCmdWaitResp = 0;
                                PT_EXIT( pt );
                        };
                        //
                        //-------------  считать строку
                        //if( resp_cnt > 0 )
                        {
                                memcpy( sms.sender_text, resp, SENDER_BUF_SZ );
                        }; 
                        //
                        //------------- OK
                        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK) && (resp_code!=RSP_ERROR) && (resp_code!=RSP_CME_ERR) && (sw_get_timeout(TIM6_SMS)>0) );
                        if( resp_code == RSP_OK )
                        {
                                modem_state.isSMSReadSuccess = 1;
                                resp_code = RSP_NOTHING;
                        };
                        //                                
                }else // parser error
                {
                        //------------- OK
                        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK) && (resp_code!=RSP_ERROR) && (resp_code!=RSP_CME_ERR) && (sw_get_timeout(TIM6_SMS)>0) );
                        if( resp_code == RSP_OK )
                        {
                                modem_state.isSMSReadSuccess = 0;
                                resp_code = RSP_NOTHING;
                        };
                };
                //
                modem_state.isCmdWaitResp = 0;
                
        };
        
        modem_state.isCmdWaitResp = 0;
        if(sw_get_timeout(TIM6_SMS)==0)
        {
                //modem_state.isSMSRxBusy = 0; // unlock SMS chanel
                //resp_code = RSP_NOTHING;
                PT_EXIT( pt );
        };
        if( (resp_code==RSP_ERROR) || (resp_code==RSP_CME_ERR) )
        { 
                //modem_state.isSMSRxBusy = 0; // unlock SMS chanel
                resp_code = RSP_NOTHING;
                PT_EXIT( pt );
        };
        // 
                        
        //------------- Удалить это сообщение
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CMGD=", TIM6_SMS,  25*10 );
        sim_put_int( modem_state.SMS_Idx );
        sim_putsf( "\r" );
        
        //------------- OK
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK) && (resp_code!=RSP_ERROR) && (resp_code!=RSP_CMS_ERR) && (sw_get_timeout(TIM6_SMS)>0) ); // OK
        modem_state.isCmdWaitResp = 0;
        if( (resp_code==RSP_OK) || 
                (resp_code==RSP_ERROR) || 
                (resp_code==RSP_CMS_ERR) )
        { 
                resp_code = RSP_NOTHING;
        };       
        //
        //modem_state.isSMSRxBusy = 0;
        //
               
        PT_END(pt);
        
}

/*-------------------------------------------------------
Послать SMS 
-------------------------------------------------------*/
struct pt sms_send_pt;

PT_THREAD(sms_send_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        //
        PT_WAIT_WHILE( pt, modem_state.isSMSRxBusy || modem_state.isCmdWaitResp );
        //
        modem_state.isSMSTxBusy = 1;
        modem_state.isSMSSendSuccess = 0;
        //
        //------------- AT test
        PT_SPAWN( pt, &child_sms_pt, AT_Test_thread(&child_sms_pt, TIM6_SMS) );
        if( modem_state.isATResponse == 0 ) // error ?
        {
                modem_state.isSMSTxBusy = 0;
                PT_EXIT( pt );
        };
        //
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        isDataPrompt = FALSE;
        sim_putsf("AT+CMGS=\"");
        sim_puts( &sms.to_send_number[0] );
        sim_putsf( "\"\r" );
        sw_set_timeout( TIM6_SMS, 60*10 );
        
        PT_WAIT_WHILE( pt, (isDataPrompt==FALSE)&&(resp_code!=RSP_CMS_ERR)&&(sw_get_timeout(TIM6_SMS)>0) ); // ждать приглашение к вводу текста
        if( sw_get_timeout(TIM6_SMS) == 0 ) // timer expired
        { 
                modem_state.isSMSTxBusy = 0;
                modem_state.isCmdWaitResp = 0;
                PT_EXIT( pt ); // error ?
        }else if( resp_code == RSP_CMS_ERR )
        {
                resp_code = RSP_NOTHING;
                modem_state.isSMSTxBusy = 0;
                modem_state.isCmdWaitResp = 0;
                PT_EXIT( pt ); // error ?
        }else  // sms data enter ready
        { 
                resp_code = RSP_NOTHING;
                // SMS text
                sim_puts( &sms.to_send_str[0] );
                sim_putsf( "\x1A" ); // CTRL + Z
                //
                PT_WAIT_WHILE( pt, (resp_code!=RSP_CMGS)&&(resp_code==RSP_CMS_ERR)&&(sw_get_timeout(TIM6_SMS)>0) ); // +CMGS = число отправляемых символов
                
                if( sw_get_timeout(TIM6_SMS) == 0 )
                { 
                        modem_state.isSMSTxBusy = 0;
                        modem_state.isCmdWaitResp = 0;
                        PT_EXIT( pt );
                }else if( resp_code==RSP_CMS_ERR )
                {
                        resp_code = RSP_NOTHING;
                        modem_state.isSMSTxBusy = 0;
                        modem_state.isCmdWaitResp = 0;
                        PT_EXIT( pt );
                }else // +CMGS
                { 
                        resp_code = RSP_NOTHING;
                        //
                        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK) && (sw_get_timeout(TIM6_SMS)>0) ); // OK = сообщение отправлено
                        if( sw_get_timeout(TIM6_SMS) == 0 )
                        { 
                                modem_state.isSMSTxBusy = 0;
                                modem_state.isCmdWaitResp = 0;
                                PT_EXIT( pt );
                        }else // OK
                        {
                                resp_code = RSP_NOTHING;
                                modem_state.isSMSSendSuccess = 1;
                                modem_state.isCmdWaitResp = 0; 
                        };
                };        
        }; 
        //
        modem_state.isSMSTxBusy = 0;
        modem_state.isCmdWaitResp = 0;
        //
        PT_END(pt);
}


/*-------------------------------------------------------
connect to server via tcp protocol
-------------------------------------------------------*/
#define CONNECTION_APN "internet.beeline.uz"
#define CONNECTION_USERNAME "beeline" 
#define CONNECTION_PASSWORD "beeline" 

#define SERVER_ADRESS   "123.123.123.123"
#define SERVER_PORT     "8500"
#define SERVER_PROTOCOL "TCP"

eeprom uint8_t apn_name[128]= "\"" 
CONNECTION_APN 
"\",\"" 
CONNECTION_USERNAME 
"\",\"" 
CONNECTION_PASSWORD 
"\"";

eeprom uint8_t server_name[100]= SERVER_ADRESS;

uint8_t tcp_at_err_cnt = 0;
uint8_t at_synch_cnt = 0;

static struct pt tcp_connect_pt;
static struct pt tcp_connect_child_pt;
static struct pt tcp_data_child_pt;
static struct pt tcp_transfer_pt;

// TCP data transfer between client and server
// TCP must be inited before using function
static PT_THREAD(tcp_data_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        //
        modem_state.isTCPRxAccepted = 0;
        modem_state.isTCPTxRq = 0;
        modem_state.isTCPInitBusy = 0;
        //
        do{
            
            PT_YIELD( pt );
            //
            if( (resp_code==RSP_CLOSE)||(resp_code==RSP_CLOSE_OK)||(resp_code==RSP_CLOSED) )
            {   // соединение прервано
                resp_code = RSP_NOTHING;
                modem_state.TCPState = TCP_CLOSED;
                break;    
            };
            //
            //
            if( modem_state.isTCPTxRq ) // есть данные на передачу
            {
                //
                PT_WAIT_WHILE( pt, modem_state.isSMSRxBusy || modem_state.isSMSTxBusy );
                //
                //------------- AT test -- попытка синхронизации с модемом
                tcp_at_err_cnt = 0;
                do{
                        PT_SPAWN( pt, &tcp_data_child_pt, AT_Test_thread(&tcp_data_child_pt, TIM7_TCP) );
                        tcp_at_err_cnt++;
                        if( modem_state.isATResponse != 0 )
                        {
                                break; // exit AT test
                        };
                }while( tcp_at_err_cnt < 4 );
                // --------------------
                //
                if( modem_state.isATResponse == 0 ) // error ?
                {
                        modem_state.isTCPTxRq = FALSE;
                        modem_state.TCPState = TCP_ERROR;
                        modem_state.isCmdWaitResp = 0;
                        break; // exit
                }else
                {   // AT synch OK
                    modem_state.isCmdWaitResp = 1;
                    //modem_state.isTCPTxBusy = 1;
                    modem_state.isTCPSentSuccess = 0;
                    isDataPrompt = FALSE;
                    modem_state.isTCPTxDataMode = 0;  // 0=не сообщать длину строоки, 1=фиксированная длина строки
                    if( modem_state.isTCPTxDataMode )
                    { 
                        strcatf( modem_state.TCPTxStr, "\r\n" );
                        modem_state.TCPTxDataSz = strlen(modem_state.TCPTxStr);
                        //
                        sim_putsf( "AT+CIPSEND=" );
                        sim_put_int( modem_state.TCPTxDataSz ); // длина строки в символах
                        sim_putsf( "\r" );
                        sw_set_timeout( TIM7_TCP, 645*10 );    
                    }else
                    {
                        sim_send_cmd( "AT+CIPSEND\r", TIM7_TCP,  645*10 ); // 645 sec
                    };                                                               
                    PT_WAIT_WHILE( pt, (isDataPrompt==FALSE)&&
                            (resp_code!=RSP_ERROR)   &&
                            (resp_code!=RSP_CME_ERR) &&
                            (resp_code!=RSP_CLOSE)   &&
                            (resp_code!=RSP_CLOSED)  &&
                            (resp_code!=RSP_CLOSE_OK)&&
                            (modem_state.TCPState==TCP_CONNECTED)&&
                            (sw_get_timeout(TIM7_TCP)>0) ); // ждать приглашение ввода строки
                    if( (resp_code == RSP_ERROR)     ||
                            (resp_code==RSP_CLOSE)   ||
                            (resp_code==RSP_CLOSED)  ||
                            (resp_code==RSP_CLOSE_OK)||
                            (resp_code==RSP_CME_ERR) ||
                            (modem_state.TCPState!=TCP_CONNECTED)||
                            (sw_get_timeout(TIM7_TCP)==0) )
                    {       // соединение преждевременно закрыто
                            resp_code = RSP_NOTHING;
                            modem_state.TCPState = TCP_ERROR;
                            modem_state.isCmdWaitResp = 0;
                            break;//PT_EXIT(pt);        
                    };
                    // приглашение получено
                    if( isDataPrompt )
                    { 
                            sim_puts( modem_state.TCPTxStr );
                            if( !modem_state.isTCPTxDataMode )
                            {       // конец строки
                                    sim_putsf( "\r\n" );
                                    sim_putsf( "\x1a" ); // ctrl + z
                            };
                            //
                            PT_WAIT_WHILE( pt, (resp_code!=RSP_SEND_OK)&&
                                    (resp_code!=RSP_ERROR)&&
                                    (resp_code!=RSP_CME_ERR)&&
                                    (resp_code!=RSP_SEND_FAIL)&&
                                    (resp_code!=RSP_CLOSE)&&
                                    (resp_code!=RSP_CLOSED)&&
                                    (resp_code!=RSP_CLOSE_OK)&&
                                    (modem_state.TCPState==TCP_CONNECTED)&&
                                    (sw_get_timeout(TIM7_TCP)>0) );
                            if( resp_code==RSP_SEND_OK )
                            { 
                                    resp_code = RSP_NOTHING;
                                    modem_state.isTCPSentSuccess = 1;
                                    //modem_state.isTCPTxBusy = 0;
                                    modem_state.isCmdWaitResp = 0;
                            }else
                            if( (resp_code == RSP_ERROR)||
                                    (resp_code==RSP_CLOSE)||
                                    (resp_code==RSP_CLOSED)||
                                    (resp_code==RSP_CLOSE_OK)||
                                    (resp_code==RSP_CME_ERR)||
                                    (resp_code==RSP_SEND_FAIL)||
                                    (modem_state.TCPState!=TCP_CONNECTED)||
                                    (sw_get_timeout(TIM7_TCP)==0) )
                            {       // соединение преждевременно закрыто
                                    resp_code = RSP_NOTHING;
                                    modem_state.TCPState = TCP_ERROR;
                                    modem_state.isCmdWaitResp = 0;
                                    break;                                
                            }else // непонятно что пришло
                            {
                                modem_state.isCmdWaitResp = 0;
                                //PT_SPAWN( pt, &tcp_data_child_pt, AT_Test_thread(&tcp_data_child_pt, TIM7_TCP) );
                                //------------- AT test -- попытка синхронизации с модемом
                                tcp_at_err_cnt = 0;
                                do{
                                        PT_SPAWN( pt, &tcp_data_child_pt, AT_Test_thread(&tcp_data_child_pt, TIM7_TCP) );
                                        tcp_at_err_cnt++;
                                        if( modem_state.isATResponse != 0 )
                                        {
                                                break; // exit AT test
                                        };
                                }while( tcp_at_err_cnt < 4 );
                                if( modem_state.isATResponse == 0 )
                                {    
                                        resp_code = RSP_NOTHING;
                                        modem_state.TCPState = TCP_ERROR;
                                        modem_state.isCmdWaitResp = 0;
                                        break; // exit AT test - нарушена синхронизация
                                };
                            };
                    }else
                    { 
                            NOP();
                            //
                            // Не получено приглашение к вводу строки
                            // неизвестное состояние
                            // возможно нарушена синхронизация
                            modem_state.isCmdWaitResp = 0;
                            //------------- AT test -- попытка синхронизации с модемом
                            tcp_at_err_cnt = 0;
                            do{
                                    PT_SPAWN( pt, &tcp_data_child_pt, AT_Test_thread(&tcp_data_child_pt, TIM7_TCP) );
                                    tcp_at_err_cnt++;
                                    if( modem_state.isATResponse != 0 )
                                    {
                                            break; // exit AT test
                                    };
                            }while( tcp_at_err_cnt < 4 );
                            /*PT_SPAWN( pt, &tcp_data_child_pt, AT_Test_thread(&tcp_data_child_pt, TIM7_TCP) );*/
                            if( modem_state.isATResponse == 0 )
                            {   
                                    resp_code = RSP_NOTHING;
                                    modem_state.TCPState = TCP_ERROR;
                                    modem_state.isCmdWaitResp = 0;
                                    break; // exit AT test - нарушена синхронизация
                            };
                    };
                    //
                };  
                //------------- передача завершена
                modem_state.isTCPTxRq = FALSE;
                modem_state.isCmdWaitResp = 0;
            };
        }while( (modem_state.TCPState==TCP_CONNECTED) && modem_state.isServerEnable );
        //
        //------------- передача завершена
        modem_state.isTCPTxRq = FALSE;
        //modem_state.isCmdWaitResp = 0;
        //
        PT_END(pt);
}

static struct pt tcp_conn_child_pt;

/*-------------------------------------------------------
AT+CIPCLOSE Close TCP or UDP Connection
-------------------------------------------------------*/
static PT_THREAD(CIPCLOSE_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        
        modem_state.isCmdWaitResp = 1;                    
        
        sim_send_cmd( "AT+CIPCLOSE\r", TIM7_TCP,  10*10 );        
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CLOSE_OK)&&
                (resp_code!=RSP_ERROR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        
        modem_state.isCmdWaitResp = 0;
        if( resp_code==RSP_CLOSE_OK )
        { 
                resp_code=RSP_NOTHING;
        };
        
        if( resp_code==RSP_ERROR )
        { 
                resp_code = RSP_NOTHING;
        };
        
        PT_END(pt);
}

/*-------------------------------------------------------
AT+CIPSHUT Deactivate GPRS PDP Context
-------------------------------------------------------*/
static PT_THREAD(CIPSHUT_thread(struct pt *pt))
{
        PT_BEGIN(pt);
        
        modem_state.isCmdWaitResp = 1;
        
        sim_send_cmd( "AT+CIPSHUT\r", TIM7_TCP,  65*10 );
        PT_WAIT_WHILE( pt, (resp_code!=RSP_SHUT_OK)&&
                (resp_code!=RSP_ERROR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
                
        modem_state.isCmdWaitResp = 0;
        if( (resp_code==RSP_SHUT_OK) )
        { 
                resp_code = RSP_NOTHING;
        };
        if(resp_code == RSP_ERROR)
        {
                resp_code = RSP_NOTHING;
        };
        
        PT_END(pt);
}

/*-------------------------------------------------------
// init TCP and connect to server
-------------------------------------------------------*/
static PT_THREAD(tcp_connect_thread(struct pt *pt))
{        
        PT_BEGIN(pt);                       
        //
        PT_WAIT_WHILE( pt, modem_state.isSMSRxBusy || modem_state.isSMSTxBusy || modem_state.isCmdWaitResp ); // ждать освобождения SMS перед посылкой потока команд
        //
        modem_state.isTCPInitBusy = 1;
        modem_state.TCPState = TCP_INIT;
        //------------- AT test
        PT_SPAWN( pt, &child_tcp_pt, AT_Test_thread(&child_tcp_pt, TIM7_TCP) );
        if( modem_state.isATResponse == 0 ) // error ?
        {
                // sync lost
                // Modem reset action
                modem_state.TCPState = TCP_ERROR;
                if( ++at_synch_cnt > 2 )
                {
                        sim800_at_init(); // restart
                };
                modem_state.isTCPInitBusy = 0;
                PT_RESTART( pt );
        };
        at_synch_cnt = 0;
        //
        modem_state.isOnLine = 0;
        //
        //PT_SPAWN( pt, &tcp_conn_child_pt, CIPCLOSE_thread(&tcp_conn_child_pt) ); // AT+CIPCLOSE
        //
        PT_SPAWN( pt, &tcp_conn_child_pt, CIPSHUT_thread(&tcp_conn_child_pt) ); // AT+CIPSHUT
        //
        // ------------- Detach from GPRS service
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;       
        sim_send_cmd( "AT+CGATT=0\r", TIM7_TCP,  75*10 ); // 
        PT_WAIT_WHILE( pt, (sw_get_timeout(TIM7_TCP)>0)&&
                (resp_code!=RSP_OK)&&
                (resp_code!=RSP_CME_ERR) );
        modem_state.isCmdWaitResp = 0;
        if( sw_get_timeout(TIM7_TCP) == 0 ) // timeout=75sec
        { 
                modem_state.isTCPInitBusy = 0;
                PT_RESTART( pt );
        }else if( (resp_code==RSP_CME_ERR)||(resp_code!=RSP_OK) )
        {
                resp_code = RSP_NOTHING;
                modem_state.isTCPInitBusy = 0;
                PT_RESTART( pt );
        }else if( resp_code == RSP_OK )
        { 
                resp_code = RSP_NOTHING;
                modem_state.isGPRSAttached = 0;
        }; 
        //
        PT_YIELD( pt );
        // ------------- Attach GPRS service
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CGATT=1\r", TIM7_TCP,  75*10 ); // 
        PT_WAIT_WHILE( pt, (sw_get_timeout(TIM7_TCP)>0)&&
                (resp_code!=RSP_OK)     &&
                (resp_code!=RSP_CME_ERR)&&
                (resp_code!=RSP_ERROR) );
        modem_state.isCmdWaitResp = 0;
        if( sw_get_timeout(TIM7_TCP) == 0 ) // timeout=75sec
        { 
                goto CIPCLOSE;
                //modem_state.TCPState = TCP_ERROR;
        }else if( (resp_code!=RSP_OK) )
        {
                resp_code = RSP_NOTHING;
                sw_set_timeout( TIM7_TCP, 5 ); // полсекунды интервал дадим чтоб не мучить модем запросами
                PT_WAIT_WHILE( pt, (sw_get_timeout(TIM7_TCP)>0) );
                goto CIPCLOSE;
                //modem_state.TCPState = TCP_ERROR;
        }else if( resp_code == RSP_OK )
        { 
                resp_code = RSP_NOTHING;
                //modem_state.isGPRSAttached = 1;
        }; 
        //
        PT_YIELD( pt );
        // ------------- GPRS service status
        do{     // надо ли крутить цикл????
                PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
                modem_state.isCmdWaitResp = 1;
                sim_send_cmd("AT+CGATT?\r", TIM7_TCP,  75*10 );
                PT_WAIT_WHILE( pt, (resp_code!=RSP_CGATT)&&(sw_get_timeout(TIM7_TCP)>0) );
                
                if( sw_get_timeout(TIM7_TCP)==0 )
                { 
                        modem_state.isCmdWaitResp = 0;
                        goto CIPCLOSE;
                        //modem_state.TCPState = TCP_ERROR;
                };
                if( resp_code == RSP_CGATT )
                { 
                        resp_code = RSP_NOTHING;
                        if( strcmpf(resp, "+CGATT: 1") == 0 ) // GPRS attached OK
                        {
                                modem_state.isGPRSAttached = 1;
                        };
                        // OK?
                        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&(resp_code!=RSP_ERROR)&&(sw_get_timeout(TIM7_TCP)>0) );
                        
                        if( resp_code==RSP_ERROR )
                        { 
                                resp_code = RSP_NOTHING;
                                modem_state.isCmdWaitResp = 0;
                                goto CIPCLOSE;
                                //modem_state.TCPState = TCP_ERROR;
                        };
                        if( resp_code==RSP_OK )
                        { 
                                resp_code = RSP_NOTHING;
                                modem_state.isCmdWaitResp = 0;
                        };
                }else // ошибка не CGATT
                {
                };
                //
                modem_state.isCmdWaitResp = 0;
                if( modem_state.isGPRSAttached == 0 )
                {
                    sw_set_timeout( TIM7_TCP, 5 ); // задержка при неготовности радиоканала
                    PT_WAIT_WHILE( pt, sw_get_timeout(TIM7_TCP)>0 );
                };
                
        }while(modem_state.isGPRSAttached == 0);
        
        PT_YIELD( pt );
        // ------------- Start task and set APN, username, password
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_putsf( "AT+CSTT=" );
        sim_putsee( apn_name, sizeof(apn_name) );
        sim_putsf( "\r" );
        
        //
        sw_set_timeout( TIM7_TCP, 10*10 );
        //
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&
                (resp_code!=RSP_ERROR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        modem_state.isCmdWaitResp = 0;
        if( resp_code == RSP_OK )
        { 
                resp_code = RSP_NOTHING;
        }else if( resp_code == RSP_ERROR )
        {
                resp_code = RSP_NOTHING;
                PT_RESTART( pt );
        };
        //
        PT_YIELD( pt );
        // ------------- Bring Up Wireless Connection with GPRS or CSD
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CIICR\r", TIM7_TCP,  85*10 ); // 85sec 
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&
                (resp_code!=RSP_ERROR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        modem_state.isCmdWaitResp = 0;
        if( resp_code == RSP_ERROR )
        { 
                resp_code = RSP_NOTHING;
                goto CIPCLOSE;
                //modem_state.TCPState = TCP_ERROR;
        }else if( resp_code == RSP_OK )
        { 
                resp_code = RSP_NOTHING;
        };
        
        
        // ------------- Get local IP address
        PT_YIELD( pt );
        //                            
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CIFSR\r", TIM7_TCP,  10*10 );
        PT_WAIT_WHILE( pt, (!(resp_str_capture &&(resp_code==RSP_NOTHING)))&&(sw_get_timeout(TIM7_TCP)>0) ); // прочитать свой IPv4
        // resp=<IPadr> действительно только когда активирован PDP контекст, иначе возврат ERROR 
        // без таймаута
        modem_state.isCmdWaitResp = 0;
        if( resp_code == RSP_ERROR )
        { 
                resp_code = RSP_NOTHING;
                goto CIPCLOSE;
        };
        
        
        // ------------- Rx data Header
        PT_YIELD( pt );
        //
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_send_cmd( "AT+CIPHEAD=1\r", TIM7_TCP,  1*10 );
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&
                (resp_code!=RSP_ERROR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        modem_state.isCmdWaitResp = 0;
        if( resp_code == RSP_OK )
        {
                resp_code = RSP_NOTHING;
        }else if( resp_code == RSP_ERROR )
        {
                resp_code = RSP_NOTHING;
                goto CIPCLOSE;        
        };
        if( sw_get_timeout(TIM7_TCP)==0 )
        {
                resp_code = RSP_NOTHING;
                goto CIPCLOSE;
        };
        
        PT_YIELD( pt );
        // ------------- Start up the connection
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        sim_putsf( "AT+CIPSTART=\"" );
        sim_putsf( SERVER_PROTOCOL );
        sim_putsf( "\",\"" );
        //sim_putsf( SERVER_ADRESS );
        sim_putsee( server_name, sizeof(server_name) );
        sim_putsf( "\",\"" );
        sim_putsf( SERVER_PORT );
        sim_putsf( "\"\r" );
        //
        sw_set_timeout( TIM7_TCP, 160*10 ); // 160sec
        PT_WAIT_WHILE( pt, (resp_code!=RSP_OK)&&
                (resp_code!=RSP_ERROR)&&
                (resp_code!=RSP_CME_ERR)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        modem_state.isCmdWaitResp = 0;
        if( resp_code != RSP_OK ) // ok
        { 
                resp_code = RSP_NOTHING;
                modem_state.isCmdWaitResp = 0;
                goto CIPCLOSE;
                modem_state.TCPState = TCP_ERROR;
        }else
        { 
                resp_code = RSP_NOTHING;
        };
        //PT_YIELD( pt );
        //
        PT_WAIT_WHILE( pt, (resp_code!=RSP_CME_ERR)&&
                (resp_code!=RSP_ALREADY_CONNECT)&&
                (resp_code!=RSP_CONNECT_OK)&&
                (resp_code!=RSP_CONNECT_FAIL)&&
                (sw_get_timeout(TIM7_TCP)>0) );
        modem_state.isCmdWaitResp = 0; // надо ли ждать?
        if( sw_get_timeout(TIM7_TCP)==0 ) // connect ok
        {
                goto CIPCLOSE;
                //modem_state.TCPState = TCP_ERROR;
        };

        
        if( (resp_code == RSP_CME_ERR) || (resp_code==RSP_CONNECT_FAIL) )
        { 
                resp_code = RSP_NOTHING;
                goto CIPCLOSE;
               // modem_state.TCPState = TCP_ERROR;
        };
        if( (resp_code==RSP_ALREADY_CONNECT)||(resp_code==RSP_CONNECT)||(resp_code==RSP_CONNECT_OK) )
        { 
                modem_state.isOnLine = 1;
                resp_code = RSP_NOTHING;
        };                              
        modem_state.TCPState = TCP_CONNECTED; // соединение установлено
        
        modem_state.TCPInitCnt = 0;        
        // ------------- Data transfer from/to server
        // непрозрачный обмен, строки данных сопровождаются командами
        PT_SPAWN( pt, &tcp_transfer_pt, tcp_data_thread(&tcp_transfer_pt) ); 
        
        // ------------- Close TCP connection
CIPCLOSE:
        PT_YIELD( pt );
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isTCPTxRq = 0;
        //modem_state.isTCPTxBusy = 0;
        modem_state.isOnLine = 0;
        // CIPCLOSE
        PT_SPAWN( pt, &tcp_conn_child_pt, CIPCLOSE_thread(&tcp_conn_child_pt) ); // AT+CIPCLOSE
        //
        PT_YIELD( pt );
        // ------------- Deactivate GPRS PDP Context
        PT_WAIT_WHILE( pt, modem_state.isCmdWaitResp );
        modem_state.isCmdWaitResp = 1;
        // CIPSHUP
        PT_SPAWN( pt, &tcp_conn_child_pt, CIPSHUT_thread(&tcp_conn_child_pt) ); // AT+CIPSHUT 
        //
        modem_state.TCPState = TCP_CLOSED;
        modem_state.isTCPTxRq = FALSE;
        // 
        modem_state.TCPInitCnt++;
        if( modem_state.TCPInitCnt > 24 ) // ~2 min interval
        { 
                modem_state.TCPInitCnt = 24;
        };
        sw_set_timeout( TIM7_TCP, ((uint16_t) modem_state.TCPInitCnt)*5 ); // 10sec задержка чтобы постоянно не мучать GPRS канал подключениями
        PT_WAIT_WHILE( pt, (sw_get_timeout(TIM7_TCP)>0) );
        //
        PT_END(pt);
        //
}

/*-------------------------------------------------------
URC response process from SIM800
-------------------------------------------------------*/
void sim_urc_resp_proc( void )
{

}


/*-------------------------------------------------------

-------------------------------------------------------*/
extern queue_t sim_rx_stream;
extern uint8_t need_server_connect;
//uint8_t resp_crlf_cnt = 0;
//uint8_t resp_str_capture = FALSE;

void sim800_proc( void )
{
        uint8_t b;
        int16_t tmp16;
        
        
        // sim read line *** response ***
        resp_str_capture = FALSE;
        resp_len = 0;
        isDataPrompt = FALSE;
        //
        //if( buf_get_cnt( &sim_rx_stream )>0 )
        if( sim_rx_stream.cnt > 0 )
        {       // из асинхронного потока приёма забираем в буфер ответа
                // буфер ответа нарезается на строки по CR-LF
                // затем буфер протаскивается через все заинтересованные потоки
                b = buf_get_data( &sim_rx_stream ); 
                resp_str[ resp_byte_cnt ] = b;
                resp_byte_cnt++;
                resp_str[ resp_byte_cnt ] = 0; // str terminator
                //resp_byte_cnt++;
                switch( resp_str_state )
                { 
                        case 0: // CR
                                if( b == '\r' )
                                {
                                        resp_str_state = 1;// next LF
                                };
                                break;
                        case 1: // LF
                                if( b == '\n' )
                                {
                                    // CR+LF закрывает строку
                                    if( resp_byte_cnt > 2 )
                                    { 
                                            resp = &resp_str[0];
                                            resp_str_capture = TRUE;
                                            resp_len = resp_byte_cnt;
                                            resp_str[ resp_byte_cnt-2 ] = 0; // str terminator
                                            resp_byte_cnt = 0;
                                            resp_str_state = 0;
                                    }else
                                    {   // CR+LF открывает строку
                                        resp_byte_cnt = 0;
                                        resp_str_state = 2; // Rx string
                                    };
                                }else if( b != '\r' ) // 
                                {       // uknown state
                                        resp_str_state = 0; // wait CR
                                };
                                break;
                        case 2: // data
                                // CR
                                if( b == '\r' )
                                {
                                        resp_str_state = 1;// next LF
                                };
                                if( resp_byte_cnt == 2 )
                                { 
                                        if( strcmpf( resp_str, "> " ) == 0 )
                                        { 
                                                isDataPrompt = TRUE;
                                        };
                                };
                                break;
                };
                
        };
        //
        
        resp_code = RSP_NOTHING;
        //
        if( resp_str_capture || isDataPrompt )
        { 
                //resp = sim_readline(); // считать одну строку ответа из буфера
                resp_code = resp_cmp( resp, RespStrCodes, RSP_TOTAL_ITEMS ); // сравнить и получить код ответа
                // ------------ обработка URC кодов ---------------------
                //sim_urc_resp_proc();
                switch( resp_code )
                { 
                        case RSP_OK:
                                modem_state.isOK = 1;
                                break;
                        case RSP_RDY:
                                break;
                        case RSP_CALL_READY: // init urc code
                                modem_state.isCallReady = TRUE;
                                resp_code = RSP_NOTHING;
                                break;
                        case RSP_SMS_READY: // init urc code
                                modem_state.isSMSReady = TRUE;
                                resp_code = RSP_NOTHING;
                                break;
                        case RSP_CMTI: // new sms arrived
                                modem_state.isSMSReady = TRUE;
                                // +CMTI: <mem>,<index>
                                // mem = "SM", "SE", "SM_P", "ME_P", "MT"
                                // параметр парсера: перепрыгнуть через 1 поле, резделитель запятая(,)
                                if( parse_int( resp, 1, 3, ',' , &modem_state.SMS_Idx ) !=0  ) // parse OK
                                {  
                                        modem_state.isSMSArrived = TRUE;
                                        PT_INIT( &sms_read_pt ); // перезапустить поток чтения смс                                        
                                };
                                resp_code = RSP_NOTHING;
                                break;
                        case RSP_CGATT:
                                // +CGATT:<state>
                                parse_int( resp, 1, 1, ':', &tmp16 );
                                modem_state.CGATT_Code = tmp16; 
                                break;
                        case RSP_IPD: // +IPD: TCP data rx  
                                //if( resp_code == RSP_IPD ) // URC пришли данные из интернета
                                {
                                    resp_code = RSP_NOTHING;
                                    parse_int( resp, 1, 3, ',' , &modem_state.TCPRxSz ); // после первого появления ',' парсить макс 3 знака
                                    if( modem_state.TCPRxSz > 0 )
                                    {
                                        modem_state.pTCPRxData = strchr( resp, ':' ); // данные начинаются после двоеточия
                                        if( modem_state.pTCPRxData != NULL )
                                        { 
                                                modem_state.pTCPRxData++;
                                                strcpy( modem_state.TCPRxStr, modem_state.pTCPRxData );
                                                modem_state.isTCPRxAccepted = 1;
                                        };
                                    };   
                                    // новые данные находятся в буфере *resp[]
                                };
                                break;  
                };
                // ------------------------------------------------------           
        };
        
        // ------------------- полная инициализация модема после вкл или ошибки  
        if( !modem_state.isModemInit )
        {
                sim_init_thread( &sim_init_pt );
                if( modem_state.isModemInit )
                {
                        modem_state.isServerEnable = ee_isServerEnable;
                        if( modem_state.isServerEnable )
                                need_server_connect = TRUE;
                };
        }else // 
        { 
        };
        
        // --------------------- SMS
        if( modem_state.isSMSReady )
        {
                sms_cmd_thread( &sms_cmd_pt );
        };
        
        // ------------------- подключение к серверу через GPRS
        if( need_server_connect )
        {
                tcp_connect_thread( &tcp_connect_pt );
                server_proc();
                if( modem_state.isServerEnable == 0 )
                { 
                        if( modem_state.TCPState == TCP_CLOSED )
                        { // ждать завершения сеанса, затем выключить обращение к серверу
                                need_server_connect = 0;
                        };
                };
        };
        //

        
        
        //
        
}

/*-------------------------------------------------------

-------------------------------------------------------*/
#include "sim800\server_proc.h"


void sim800_at_init( void )
{
        PT_INIT( &sim_init_pt );
        PT_INIT( &sms_send_pt );
        PT_INIT( &sms_read_pt );
        PT_INIT( &sms_cmd_pt  );         
        
        memset( &modem_state, 0, sizeof(modem_state) ); // сбросить все флажки
        
        modem_state.TCPState = TCP_INIT;
        
        need_server_connect=FALSE;
        
        modem_state.isCmdWaitResp = 0;
        
}