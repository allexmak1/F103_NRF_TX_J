#include "main.h"
#include "stm32f1xx_hal.h"

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
//0000 0000 0000 0000 0000 0111 0000 0111

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
    
    uint8_t osn   :4;//3.0
    uint8_t dop   :4;//3.4
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
uint8_t vToogleLedLow(uint8_t led);
int map_i (int x, int in_min, int in_max, int out_min, int out_max);
void vVisionLedBar(int mode, int value);
void vLedBar(int mode, int n);
void vToogleLed4();