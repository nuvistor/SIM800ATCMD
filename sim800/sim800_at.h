/*-------------------------------------------------------

-------------------------------------------------------*/
#ifndef SIM800_AT_H
#define SIM800_AT_H

#include <stdint.h>

/*-------------------------------------------------------

-------------------------------------------------------*/
typedef enum{
TCP_INIT=0,
TCP_CONNECTED,
TCP_CLOSED,
TCP_ERROR
} tcp_conn_state_t;

/*-------------------------------------------------------

-------------------------------------------------------*/
typedef struct{
        uint8_t isSynch;//:1;  // начальная синхронизация
        uint8_t isModemInit;//:1;
        uint8_t isPinReady;//:1;
        uint8_t isRegistered;//:1;
        uint8_t CREGSatus;
        
        uint8_t isCmdWaitResp; // ждём ответа команды, не предпринимаем действий чтобы не нарушать синхронизацию
        
        uint8_t isOK; // OK response
        
        uint8_t isCallReady;//:1; // Call Ready
        uint8_t isSMSReady;//:1;  // SMS Ready
        //
        uint8_t isCMEErr;//:1;  // +CME ERROR: <err>
        uint8_t CMEErrCode;
        //              
        uint8_t isCMSErr;//:1;  // +CMS ERROR: <err>
        uint8_t CMSErrCode;//:1;
        // SMS
        uint8_t isSMSArrived;//:1; // +CMTI: "SM",   -- new sms arrived
        int16_t SMS_Idx; // new SMS index
        //
        uint8_t isSMSRxBusy;//:1; // модем занят приёмом или передачей данных
        uint8_t isSMSTxBusy;//:1; // если в этот момент загружать его другими командами, может произойти
        //                        // рассинхронизация или зависание модуля                     
        uint8_t isSMSReadSuccess;//:1; // SMS успешно прочитано
        //
        uint8_t isSMS_SendRq; // запрос на отправку СМС - строка подготовлена
        uint8_t isSMSSendSuccess;//:1; // смс успешно послано
        //
        uint8_t isGPRSAttached;//:1;
        // GPRS
        uint8_t CGATT_Code;
        // TCP
        uint8_t TCPInitCnt; 
        uint8_t isTCPInitBusy;//:1; //идёт процесс инициализации соединения
        uint8_t isTCPRxBusy;//:1;
        //uint8_t isTCPTxBusy;//:1; // TCP канал занят - началась подготовка или начало отправки данных. Другие процессы не должны слать данные в модем
        uint8_t isTCPTxRq;//:1; // request to send buf
        uint8_t isTCPSentSuccess;//:1; // успешно отправлены данные
        uint8_t *pTCPRxData; // ptr to TCP RX st buf
        uint8_t TCPRxStr[256];
        uint8_t isTCPRxAccepted;//:1; // new data received
        int16_t TCPRxSz; // размер полученных данных
        uint8_t TCPTxStr[256]; // буфер(строка) передачи от клиента к серверу
        uint16_t TCPTxDataSz; 
        uint8_t isTCPTxDataMode; // 0=str, 1=data
        tcp_conn_state_t TCPState;
        //
        uint8_t isOnLine;//:1;
        //
        uint8_t isATResponse;//:1; // OK response to AT cmd
        uint8_t isServerEnable;
        
} modem_state_t;

extern modem_state_t modem_state; //

/*-------------------------------------------------------

-------------------------------------------------------*/
#define SENDER_BUF_SZ   256

typedef struct{ // SMS buffers
        uint8_t to_send_number[20];
        uint8_t to_send_str[SENDER_BUF_SZ];
        //
        uint8_t sender_number[20];
        uint8_t sender_text[SENDER_BUF_SZ];
}sms_t;

extern sms_t sms;


/*-------------------------------------------------------

-------------------------------------------------------*/
/*typedef enum{
CONN_INIT=0,
CONN_OK,
CONN_CLOSED,
CONN_ERROR
} tcp_conn_state_t; */




/*-------------------------------------------------------

-------------------------------------------------------*/

void sim800_at_init( void );
void sim800_proc( void );

#endif  SIM800_AT_H
