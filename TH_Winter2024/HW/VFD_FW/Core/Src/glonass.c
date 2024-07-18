#include "glonass.h"
#include "main.h"
#include "work_logic.h"
//#include "..\clock\xtime.h"
#include <stdlib.h>
#include <string.h>


extern UART_HandleTypeDef     huart1;              //GLONASS 0

volatile uint8_t              NMEACount;           // Счётчик принятых от GPS'а символов
volatile uint8_t              NMEAFlag;            // Флаг того, что пакет принят

extern GPS_Data_Struct               GPS_Data;


//--------------------------------------------------------------------------------------------------

// in stm32f1xx_it.c:
/*
void USART1_IRQHandler(void)
{
//   USER CODE BEGIN USART1_IRQn 0 
   GLNS_UART_IRQHandler(&huart1);
//  USER CODE END USART1_IRQn 0 

*/


//--------------------------------------------------------------------------------------------------

// In PreLoop

/*
   GLNS_InitStr();
   //IRQ enable
   huart1.Instance->SR &= ~ (USART_SR_TC | USART_SR_RXNE);
   huart1.Instance->DR;
   huart1.Instance->CR1 |= USART_CR1_RXNEIE;

*/
// In Loop
/*
      if (NMEAFlag == MSG_RMC){
         GLNS_NMEADataConvertRMC();

         Latitude_GLONASS   = GPS_Data.Lat;
         Longtitude_GLONASS = GPS_Data.Long;

         if(GPS_Data.Valid){
            start_time = HAL_GetTick();
         }
         else {
            //do reboot GLONASS protothread when 'valid' flag is lost
            if((HAL_GetTick() - start_time) > GLONASS_VALID_TIMEOUT){
               start_time = HAL_GetTick();
               PT_EXIT(pt);
            }
         }

         //TMI5

         uint8_t hour     = GPS_Data.Hour;
         uint8_t min      = GPS_Data.Min;
         uint8_t sec      = GPS_Data.Sec;

         // Сбросить флаг приёма посылки
         NMEAFlag = 0;
         NMEACount = 0;
         //GPS_Data.pack_bufNum = 0;
      }
      if (NMEAFlag == MSG_GGA){
         
         GLNS_NMEADataConvertGGA();
         
         NMEAFlag = 0;
         NMEACount = 0;
      }
*/

//--------------------------------------------------------------------------------------------------

void GLNS_NMEADataConvertRMC(void)      // Функция расшифровки NMEA посылок
{
   long        gr, mn, mhi, mlo;

   // $GPRMC,000052.036,V,5503.3587,N,08257.8564,E,0.00,0.00,060180,,,N*40

   // Вытащить время
   if (GLNS_NMEAGetParam(1) > 5)
   {
      GPS_Data.Hour = (GPS_Data.par_buf[0] - 0x30) * 10 + (GPS_Data.par_buf[1] - 0x30);
      GPS_Data.Min  = (GPS_Data.par_buf[2] - 0x30) * 10 + (GPS_Data.par_buf[3] - 0x30);
      GPS_Data.Sec  = (GPS_Data.par_buf[4] - 0x30) * 10 + (GPS_Data.par_buf[5] - 0x30);
   }

   // Вытащить широту
   if (GLNS_NMEAGetParam(3) > 7)
   {
      uint8_t LatDeg   = (GPS_Data.par_buf[0]-0x30)*10 + (GPS_Data.par_buf[1]-0x30);
      uint8_t LatMin   = (GPS_Data.par_buf[2]-0x30)*10 + (GPS_Data.par_buf[3]-0x30);
      uint8_t LatMinHi = (GPS_Data.par_buf[5]-0x30)*10 + (GPS_Data.par_buf[6]-0x30);
      uint8_t LatMinLo = (GPS_Data.par_buf[7]-0x30)*10 + (GPS_Data.par_buf[8]-0x30);
      uint8_t  NS;

      GPS_Data.Lat =    (int32_t)LatDeg   * 600000
                     +  (int32_t)LatMin   * 10000
                     +  (int32_t)LatMinHi * 100
                     +  LatMinLo;

      //sign
      if (GLNS_NMEAGetParam(4) > 0)
      {
         NS = GPS_Data.par_buf[0];
         if (NS == 'S')
            GPS_Data.Lat *= -1;
      }
   }

   // Вытащить долготу
   if (GLNS_NMEAGetParam(5) > 7)
   {
      uint8_t LongDegHi = (GPS_Data.par_buf[0]-0x30);
      uint8_t LongDeg   = (GPS_Data.par_buf[1]-0x30)*10 + (GPS_Data.par_buf[2]-0x30);
      uint8_t LongMin   = (GPS_Data.par_buf[3]-0x30)*10 + (GPS_Data.par_buf[4]-0x30);
      uint8_t LongMinHi = (GPS_Data.par_buf[6]-0x30)*10 + (GPS_Data.par_buf[7]-0x30);
      uint8_t LongMinLo = (GPS_Data.par_buf[8]-0x30)*10 + (GPS_Data.par_buf[9]-0x30);

      gr   = (long)(LongDeg);
      if (LongDegHi != 0)
         gr +=100;

      mn   = (long)(LongMin);
      mhi  = (long)(LongMinHi);
      mlo  = (long)(LongMinLo);

      GPS_Data.Long =      gr  * 600000 +
                           mn  * 10000 +
                           mhi * 100 +
                           mlo;

      if (GLNS_NMEAGetParam(6) > 0)
      {
         uint8_t EW = GPS_Data.par_buf[0];
         if (EW == 'W') GPS_Data.Long *= -1;
      }
   }
   
   // Вытащить валидность
   if (GLNS_NMEAGetParam(2) > 0)
   {
      GPS_Data.Valid = 0;
      if (GPS_Data.par_buf[0] == 'A')
         GPS_Data.Valid = 1;
   }
}

//--------------------------------------------------------------------------------------------------

void GLNS_NMEADataConvertGGA(void){
}

//--------------------------------------------------------------------------------------------------
// return -1 - если параметр не найден
// return  0 - параметр найден, но он пуст
// return [длина] - параметр найден, [длина] = кол-во вычитанных символов и в [GPS_Data.par_buf] - сам параметр

int GLNS_NMEAGetParam(int par) // par - номер параметра в посылке {1..255}
{
   int h = 0, i;
   char c;

   if (par < 1)
      return -1;

   for (i = 0; i < 80; i ++)
   {
      c = GPS_Data.pack_buf[i];
      if (((c == ',') && (++h == par )) || (c  == '*'))
         break;
   }

   if ((i < 80) && (GPS_Data.pack_buf[i] != '*'))
   {
      i ++;
      h = 0;
      while (h < 12)
      {
         c = GPS_Data.pack_buf[i++];
         if ((c == ',') || (c == '*'))
            break;
         else
            GPS_Data.par_buf[h++] = c;
      }
      GPS_Data.par_buf[h] = 0;
      return h;
   }
   else
      return -1;
}

//--------------------------------------------------------------------------------------------------

void GLNS_InitStr(void)
{
   NMEACount = 0;   // Сбросить счётчик принятых от GPS'а символов
   NMEAFlag  = 0;   // Сбросить флаг заполнения буфера приёмника

   GPS_Data.Hour = 23;
   GPS_Data.Min  = 59;
   GPS_Data.Sec  = 55;
} // InitGPS

//--------------------------------------------------------------------------------------------------

void GLNS_Parcer(uint8_t c){
   uint8_t i, calc_crc, pac_crc;

   if (NMEAFlag != 0)
      return;                                   // Если предыдущая посылка необработана, то выйти

   if ((NMEACount == 0) && (c == '$')){         // Если первый принятый символ не "$", то выйти
      GPS_Data.pack_buf[NMEACount++] = c;    // если "$", то запомнить его и увеличить счётчик принятых символов
      return;
   }

   if (NMEACount > 0){
      GPS_Data.pack_buf[NMEACount++] = c;
      if (GPS_Data.pack_buf[NMEACount - 3] == '*'){                  // end of the packet

         //CRC calculate
         calc_crc = GPS_Data.pack_buf[1];
         for(i = 2; i < (NMEACount - 3); i ++){
            calc_crc ^= GPS_Data.pack_buf[i];
         }
         GPS_Data.pack_buf[NMEACount] = 0;
         pac_crc = strtol((const char *)&GPS_Data.pack_buf[NMEACount - 2], NULL, 16);

         //CRC control
         if(pac_crc == calc_crc){

            //put the flags
            if(   (GPS_Data.pack_buf[3] == 'R') &&
                  (GPS_Data.pack_buf[4] == 'M') &&
                  (GPS_Data.pack_buf[5] == 'C')){
                     NMEAFlag = MSG_RMC;
            }
            else if( (GPS_Data.pack_buf[3] == 'G') &&
                     (GPS_Data.pack_buf[4] == 'G') &&
                     (GPS_Data.pack_buf[5] == 'A')){
                     NMEAFlag = MSG_GGA;
            }
         } else {
            NMEAFlag  = 0;
         }
         NMEACount = 0;
      }
      if (NMEACount > 79){
         NMEACount = 0;
         NMEAFlag  = 0;
      }
   }
}

//--------------------------------------------------------------------------------------------------

void GLNS_UART_IRQHandler(UART_HandleTypeDef * huart){
   volatile uint8_t data_reg;


   if (huart->Instance->SR & USART_SR_RXNE) {
      data_reg = huart->Instance->DR;
      GLNS_Parcer(data_reg);
   }

   if (huart->Instance->SR & USART_SR_ORE) {
      huart->Instance->SR;
      huart->Instance->DR;
      NMEACount = 0;
   }
}

//--------------------------------------------------------------------------------------------------

