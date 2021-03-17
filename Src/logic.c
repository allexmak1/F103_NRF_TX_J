#include "logic.h"
#include "main.h"
#include "string.h"
#include "nrf24.h"


extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

jButton_t jButton       = {0};
jStick_t  jStickA       = {0};
jStick_t  jStickB       = {0};
jLed_t    jLed          = {0};

extern uint32_t timer1msSleep;
extern uint32_t timer1msSendState;
extern uint32_t timer_Led4;

volatile uint16_t adc[4] = {0,}; // ? ??? ??? ?????? ??????? ?????? ?? 4 ?????????
volatile uint8_t flagDmaAdc = 0;



extern uint8_t nRF24_payload[32];
extern nRF24_RXResult pipe;
extern  uint8_t payload_length;
//extern uint32_t i;
extern nRF24_TXResult tx_res;

void runRadio(void);
void UART_SendStr(char *string);
void UART_SendBufHex(char *buf, uint16_t bufsize);
nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length);

void LOGICstart(){
HAL_Delay(500);
  vSetStateGpio(0, GPIOC, GPIO_PIN_13);
  vSetStateGpio(0, GPIOC, GPIO_PIN_14);
  vSetStateGpio(0, GPIOC, GPIO_PIN_15);
  vSetStateGpio(0, GPIOA, GPIO_PIN_15);
  
  HAL_TIM_Base_Start_IT(&htim1);
  //инициализация
  HAL_Delay(100);
  runRadio();
  vSetStartADC();
  payload_length = 12;
  HAL_Delay(500);
  
}


void LOGIC(){

  //чтение всех входов и выходов
  vReadStatePins();
  
  // подготовка данных
nRF24_payload[0] = jButton.uint8Message.gr1;
nRF24_payload[1] = jButton.uint8Message.gr2;;
nRF24_payload[2] = jButton.uint8Message.gr3;;
nRF24_payload[3] = jButton.uint8Message.gr4;;
nRF24_payload[4] = jStickA.ValV;
nRF24_payload[5] = jStickA.ValV >> 8;
nRF24_payload[6] = jStickA.ValG;
nRF24_payload[7] = jStickA.ValG >> 8;
nRF24_payload[8] = jStickB.ValV;
nRF24_payload[9] = jStickB.ValV >> 8;
nRF24_payload[10] = jStickB.ValG;
nRF24_payload[11] = jStickB.ValG >> 8;

  


  
  
  

  if(timer1msSendState > 200){
    timer1msSendState = 0;
    vSendStateJ();
  
  }
  
  if(timer_Led4 > 3000){
  //timer1msLed = 0;
  //jLed.one = ~jLed.one &0x01;
  //jLed.two = ~jLed.two &0x01;
  //jLed.fhree = ~jLed.fhree &0x01;
  //jLed.four = ~jLed.four &0x01;
  
  
      if(timer_Led4 > 3000 && timer_Led4 < 3010) jLed.four = 1;
    if(timer_Led4 > 3100){
      timer_Led4 = 0;
      jLed.four = 0;
    }
}

  //HAL_Delay(10);
  
}

void vReadStatePins(){
  static int temp=0;
  //светодиоды
  vSetStateGpio(jLed.one, GPIOC, GPIO_PIN_13);
  vSetStateGpio(jLed.two, GPIOC, GPIO_PIN_14);
  vSetStateGpio(jLed.fhree, GPIOC, GPIO_PIN_15);
  vSetStateGpio(jLed.four, GPIOA, GPIO_PIN_15);
  //кнопки (обрабатываем только те которые не EXTI)
  jButton.bit.Lb        = xGetStateGpio(jButton.bit.Lb,      GPIOB, GPIO_PIN_3);
  jButton.bit.Ls        = xGetStateGpio(jButton.bit.Ls,      GPIOB, GPIO_PIN_4);
  jButton.bit.home      = xGetStateGpio(jButton.bit.home,      GPIOB, GPIO_PIN_8);
  jButton.bit.list      = xGetStateGpio(jButton.bit.list,      GPIOB, GPIO_PIN_9);
  //какой то глюк (всегда на этой ноге 1)
  jButton.bit.back      = 0;//xGetStateGpio(jButton.bit.back,    GPIOA, GPIO_PIN_12);
  
  jButton.bit.StickA    = xGetStateGpio(jButton.bit.StickA,  GPIOB, GPIO_PIN_11);
  jButton.bit.StickB    = xGetStateGpio(jButton.bit.StickB,  GPIOB, GPIO_PIN_10);
  //
  jButton.bit.up        = xGetStateGpio(jButton.bit.up,  GPIOB, GPIO_PIN_12);
  jButton.bit.dawn      = xGetStateGpio(jButton.bit.dawn,  GPIOB, GPIO_PIN_13);
  jButton.bit.write     = xGetStateGpio(jButton.bit.write,  GPIOB, GPIO_PIN_14);
  jButton.bit.left      = xGetStateGpio(jButton.bit.left,  GPIOB, GPIO_PIN_15);
  jButton.bit.O         = xGetStateGpio(jButton.bit.O,  GPIOA, GPIO_PIN_8);
  
  
  //jButton.bit.X         = xGetStateGpio(jButton.bit.X,  GPIOA, GPIO_PIN_9);
  //jButton.bit.A         = xGetStateGpio(jButton.bit.A,  GPIOA, GPIO_PIN_10);
  
  jButton.bit.B         = xGetStateGpio(jButton.bit.B,  GPIOA, GPIO_PIN_11);
  jButton.bit.Wb        = xGetStateGpio(jButton.bit.Wb,  GPIOB, GPIO_PIN_5);
  jButton.bit.Ws        = xGetStateGpio(jButton.bit.Ws,  GPIOB, GPIO_PIN_6);
  jButton.bit.start     = xGetStateGpio(jButton.bit.start,  GPIOA, GPIO_PIN_0);
  jButton.bit.select    = xGetStateGpio(jButton.bit.select,  GPIOB, GPIO_PIN_7);

  //кнопки которые в EXTI смотрим только отжатие
  /*if(jButton.bit.up)    jButton.bit.up    = xGetStateGpio(jButton.bit.up,      GPIOB, GPIO_PIN_12);
  if(jButton.bit.dawn)  jButton.bit.dawn  = xGetStateGpio(jButton.bit.dawn,    GPIOB, GPIO_PIN_13);
  if(jButton.bit.write) jButton.bit.write = xGetStateGpio(jButton.bit.write,   GPIOB, GPIO_PIN_14);
  if(jButton.bit.left)  jButton.bit.left  = xGetStateGpio(jButton.bit.left,    GPIOB, GPIO_PIN_15);
  if(jButton.bit.O)     jButton.bit.O     = xGetStateGpio(jButton.bit.O,       GPIOA, GPIO_PIN_8);
  if(jButton.bit.X)     jButton.bit.X     = xGetStateGpio(jButton.bit.X,       GPIOA, GPIO_PIN_9);
  if(jButton.bit.A)     jButton.bit.A     = xGetStateGpio(jButton.bit.A,       GPIOA, GPIO_PIN_10);
  if(jButton.bit.B)     jButton.bit.B     = xGetStateGpio(jButton.bit.A,       GPIOA, GPIO_PIN_11);
  if(jButton.bit.start) jButton.bit.start = xGetStateGpio(jButton.bit.start,   GPIOA, GPIO_PIN_0);
  if(jButton.bit.select)jButton.bit.select= xGetStateGpio(jButton.bit.select,  GPIOB, GPIO_PIN_7);
  if(jButton.bit.Wb)    jButton.bit.Wb    = xGetStateGpio(jButton.bit.Wb,    GPIOB, GPIO_PIN_5);
  if(jButton.bit.Ws)    jButton.bit.Ws    = xGetStateGpio(jButton.bit.Ws,    GPIOB, GPIO_PIN_6);*/
  
  //АЦП код на все каналы
  if(flagDmaAdc)
  {
    flagDmaAdc = 0;
    HAL_ADC_Stop_DMA(&hadc1); // это необязательно
    
    //ADC_CHANNEL_1
    temp = (uint16_t)adc[0];
    temp -= jStickA.zeroValG;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickA.ValG = temp;
       }else jStickA.ValG = 0;
    
    //ADC_CHANNEL_2   
    temp = (uint16_t)adc[1];
    //приведение к нулю
    temp -= jStickA.zeroValV;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickA.ValV = temp;
       }else jStickA.ValV = 0;
    
    //ADC_CHANNEL_3
    temp = (uint16_t)adc[2];
    //приведение к нулю
    temp -= jStickB.zeroValG;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickB.ValG = temp;
       }else jStickB.ValG = 0;
    
    //ADC_CHANNEL_4
    temp = (uint16_t)adc[3];
    //приведение к нулю
    temp -= jStickB.zeroValV;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickB.ValV = temp;
       }else jStickB.ValV = 0;

    adc[0] = 0;
    adc[1] = 0;
    adc[2] = 0;
    adc[3] = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 4);
  };

}
//определение изменения состояния входа
uint8_t xGetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
  uint8_t state = stateInput;
  if(state != (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin))){
    state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    timer1msSleep=0;
  }
  return state;
}
//определение изменения состояния выхода
void vSetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
  if(stateInput == 1){
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  }else {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  }
}
void vSendStateJ(){
  
    	// Print a payload
    	UART_SendStr("PAYLOAD:>");
    	UART_SendBufHex((char *)nRF24_payload, payload_length);
    	UART_SendStr("< ... TX: ");

    	// Transmit a packet
    	tx_res = nRF24_TransmitPacket(nRF24_payload, payload_length);
    	switch (tx_res) {
			case nRF24_TX_SUCCESS:
				UART_SendStr("OK");
				break;
			case nRF24_TX_TIMEOUT:
				UART_SendStr("TIMEOUT");
				break;
			case nRF24_TX_MAXRT:
				UART_SendStr("MAX RETRANSMIT");
				break;
			default:
				UART_SendStr("ERROR");
				break;
		}
    	UART_SendStr("\r\n");
}

void vSetStartADC(){
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 16); // ???????? ???
  HAL_Delay(50);
  if(flagDmaAdc)
  {
    flagDmaAdc = 0;
    HAL_ADC_Stop_DMA(&hadc1); // ??? ?????????????
    jStickA.zeroValV = (uint16_t)adc[0];
    jStickA.zeroValG = (uint16_t)adc[1];
    jStickB.zeroValV = (uint16_t)adc[2];
    jStickB.zeroValG = (uint16_t)adc[3];
    adc[0] = 0;
    adc[1] = 0;
    adc[2] = 0;
    adc[3] = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 16);
  };
}




















