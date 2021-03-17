//распиновка
/*
PB12- up*
PB13- dawn*
PB14- write*
PB15- left*
PA8 - O*
PA9 - X*
PA10- A*
PA11- B*
PA12- back--не работает
PA15- LED_4
PB3 - Lb
PB4 - Ls
PB5 - Wb*
PB6 - Ws*
PB7 - select*
PB8 - home
PB9 - list
5V
GND
3V3
//------
GND
GND
3V3
NRST
PB11- jStickA.Batton 
PB10- jStickB.Batton
PB1 - SPI1_CS
PB0 - SPI1_DT
PA7 - SPI1_MOSI
PA6 - SPI1_MISO
PA5 - SPI1_SCK
PA4 - ADC1_IN4
PA3 - ADC1_IN3
PA2 - ADC1_IN2
PA1 - ADC1_IN1
PA0 - WKUP - start*
PC15- LED_3
PC14- LED_2
PC13- LED_1
//------
-non PA13-програматор
-non PA14-програматор
-non PB2 -BOOT1
*/

#include "main.h"
#include "stm32f1xx_hal.h"


#define TIMER_SLEEP 5000//ms
#define OFFSET_ZERO 180 //смещение, антидергание на нуле

//ѕередавать не больше 32-х байт 
//получаетс€ отправл€ем 12(16) байт (кнопки, стикј и стик¬)
//кнопки - 4 байта
//0000 0000 0000 0111 1111 1111 1111 1111
//                |<-           кнопки <-
//стикј - 4 байта
//0000 1111 1111 1111 0000 1111 1111 1111
//|<-         ValG <- |<-         ValV <-
//стик¬ - 4 байта
//0000 1111 1111 1111 0000 1111 1111 1111
//|<-         ValG <- |<-         ValV <-
//пустой дл€ четности - 4 байта
//0000 0000 0000 0000 0000 0000 0000 0000

//кнопки
typedef union{
  struct{
    uint8_t up    :1;//0.0
    uint8_t dawn  :1;//0.1
    uint8_t write :1;//0.2
    uint8_t left  :1;//0.3
    uint8_t O     :1;//0.4
    uint8_t X     :1;//0.5
    uint8_t A     :1;//0.6
    uint8_t B     :1;//0.7
    
    uint8_t Lb    :1;//1.0
    uint8_t Ls    :1;//1.1
    uint8_t Wb    :1;//1.2
    uint8_t Ws    :1;//1.3
    uint8_t start :1;//1.4
    uint8_t select:1;//1.5
    uint8_t home  :1;//1.6
    uint8_t back  :1;//1.7--не работает
    
    uint8_t list  :1;//2.0
    uint8_t StickA:1;//2.1
    uint8_t StickB:1;//2.2
    uint8_t       :5;//2.3
    
    uint8_t       :8;
  }bit;
  struct{
    uint8_t gr1;
    uint8_t gr2;
    uint8_t gr3;
    uint8_t gr4;
  }uint8Message;
  uint32_t uint32Message;
}jButton_t;

//стики
typedef struct{
  int16_t ValV;
  int16_t ValG;
  int16_t zeroValV;
  int16_t zeroValG;
}jStick_t;

//лампочки
typedef struct{
  uint8_t one;
  uint8_t two;
  uint8_t fhree;
  uint8_t four;
}jLed_t;

//режимы работы
typedef enum{
  MODE_1 = 1,
  MODE_2, 
  MODE_3
} mode_e;

typedef struct{
  mode_e    osn;
  mode_e    dop;
  uint8_t   isMode;
}jMode_t;

void LOGICstart();
void LOGIC();
void vReadStatePins();
uint8_t xGetStateGpio(uint8_t stateOutpt, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void vSetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void vSendStateJ();
void vSetStartADC();
void vNavigationMode();
void vToogleLedLow(uint8_t led);
int map_i (int x, int in_min, int in_max, int out_min, int out_max);