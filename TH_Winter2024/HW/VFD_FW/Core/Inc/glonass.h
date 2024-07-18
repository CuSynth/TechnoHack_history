#ifndef _GLONASS_H_
#define _GLONASS_H_

#include "stm32f1xx.h"
#include "pt.h"

//--------------------------------------------------------------------------------------------------

#define MSG_RMC                        1
#define MSG_GGA                        2

#define VALID_BIT                      0x80

#define GLONASS_VALID_TIMEOUT          120000ul

//--------------------------------------------------------------------------------------------------
// ��������� ��������� ������ ����������� �� GPS'a

#pragma pack(push, 1)

typedef struct {

   uint8_t  Hour;          // ����         {00..23}
   uint8_t  Min;           // ������      {00..59}
   uint8_t  Sec;           // �������      {00..59}

   
   uint8_t  Valid;         // ���������� ������ (0 - fix not available, 1 - GPS fix, 2 - Differential GPS fix)

   int32_t  Lat;           // ������ � �������������� ����� ������  (32 ���� �� ������, - ����� + ��������)
   int32_t  Long;          // ������� � �������������� ����� ������ (32 ���� �� ������, - �������� + ���������)   
   
   uint8_t  pack_buf[80];  // ����� ����� ������ �� �������
   uint8_t  par_buf[16];
   int32_t  leap_second;   // +140
   
} GPS_Data_Struct; 

#pragma pack(pop)

//--------------------------------------------------------------------------------------------------


void     GLNS_Machine(void);

void           GLNS_NMEADataConvertRMC(void);
void           GLNS_NMEADataConvertGGA(void);
int            GLNS_NMEAGetParam(int par);
void           GLNS_InitStr(void);
void           GLNS_Parcer(uint8_t c);
void           GLNS_UART_IRQHandler(UART_HandleTypeDef * huart);
uint32_t       GetSecondsSince2000(GPS_Data_Struct * str, int32_t sec_cor);
uint32_t       PowerOf10(uint8_t k);


#endif

