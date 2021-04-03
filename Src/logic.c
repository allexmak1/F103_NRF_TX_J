#include "logic.h"
#include "main.h"
#include "string.h"
#include "nrf24.h"
#include "config.h"

extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

jButton_t jButton       = {0};
jStick_t  jStickA       = {0};
jStick_t  jStickB       = {0};
jLed_t    jLed          = {0};
jMode_t   jMode   	= {MODE_1, MODE_1, 0};

extern uint32_t timer_Sleep;
extern uint32_t timer_SendState;
extern uint32_t timer_Led4;
extern uint32_t timer_LedLow;
extern uint32_t timer_ticBar;

volatile uint16_t       adc[5] = {0,}; // у нас два канала поэтому массив из 5 элементов
volatile uint8_t        flagDmaAdc = 0;

extern uint8_t          nRF24_payload[32];
extern nRF24_RXResult   pipe;
extern uint8_t          payload_length;
       nRF24_TXResult   stateNrfTX;
       uint8_t          countNoneNrf;
       int 	        voltageAkbADC;
       
       short indZaradAkbJ, indZaradAkbRC;
       short countIndZaradAkbJ;
       short countIndZaradAkbRC;
nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length);

// Some variables
//uint32_t packets_lost = 0; // global counter of lost packets
//uint8_t otx;
//uint8_t otx_plos_cnt; // lost packet count
//uint8_t otx_arc_cnt; // retransmit count

void LOGICstart(){
  HAL_TIM_Base_Start_IT(&htim1);
  //HAL_Delay(500);
  vSetStateGpio(0, GPIOC, GPIO_PIN_13);
  vSetStateGpio(0, GPIOC, GPIO_PIN_14);
  vSetStateGpio(0, GPIOC, GPIO_PIN_15);
  vSetStateGpio(0, GPIOA, GPIO_PIN_15);
  HAL_Delay(100);
  //инициализация NRF
  nRF24_CE_L();// RX/TX disabled
  int wile = 0;
  while(wile == 0){
    if(nRF24_Check()){
      wile = 1;
    }
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_15);
    Delay_ms(50);
  }
  nRF24_Init();
  
  // This is simple transmitter (to one logic address):
  //   - TX address: '0xE7 0x1C 0xE3'
  //   - payload: 5 bytes
  //   - RF channel: 115 (2515MHz)
  //   - data rate: 250kbps (minimum possible, to increase reception reliability)
  //   - CRC scheme: 2 byte
  // The transmitter sends a 5-byte packets to the address '0xE7 0x1C 0xE3' without Auto-ACK (ShockBurst disabled)
  // Disable ShockBurst for all RX pipes
  nRF24_DisableAA(0xFF);
  // Set RF channel
  nRF24_SetRFChannel(115);
  // Set data rate
  nRF24_SetDataRate(nRF24_DR_250kbps);
  // Set CRC scheme
  nRF24_SetCRCScheme(nRF24_CRC_2byte);
  // Set address width, its common for all pipes (RX and TX)
  nRF24_SetAddrWidth(3);
  // Configure TX PIPE
  static const uint8_t nRF24_ADDR[] = { 0xE7, 0x1C, 0xE3 };
  nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR); // program TX address
  // Set TX power (maximum)
  nRF24_SetTXPower(nRF24_TXPWR_0dBm);
  // Set operational mode (PTX == transmitter)
  nRF24_SetOperationalMode(nRF24_MODE_TX);
  // Clear any pending IRQ flags
  nRF24_ClearIRQFlags();
  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);
  
//  // This is simple transmitter with Enhanced ShockBurst (to one logic address):
//  //   - TX address: 'ESB'
//  //   - payload: 10 bytes
//  //   - RF channel: 40 (2440MHz)
//  //   - data rate: 2Mbps
//  //   - CRC scheme: 2 byte
//  
//  // The transmitter sends a 10-byte packets to the address 'ESB' with Auto-ACK (ShockBurst enabled)
//  // Set RF channel
//  nRF24_SetRFChannel(40);
//  // Set data rate
//  nRF24_SetDataRate(nRF24_DR_2Mbps);
//  // Set CRC scheme
//  nRF24_SetCRCScheme(nRF24_CRC_2byte);
//  // Set address width, its common for all pipes (RX and TX)
//  nRF24_SetAddrWidth(3);
//  // Configure TX PIPE
//  static const uint8_t nRF24_ADDR[] = { 'E', 'S', 'B' };
//  nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR); // program TX address
//  nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR); // program address for pipe#0, must be same as TX (for Auto-ACK)
//  // Set TX power (maximum)
//  nRF24_SetTXPower(nRF24_TXPWR_0dBm);
//  // Configure auto retransmit: 10 retransmissions with pause of 2500s in between
//  nRF24_SetAutoRetr(nRF24_ARD_2500us, 10);
//  // Enable Auto-ACK for pipe#0 (for ACK packets)
//  nRF24_EnableAA(nRF24_PIPE0);
//  // Set operational mode (PTX == transmitter)
//  nRF24_SetOperationalMode(nRF24_MODE_TX);
//  // Clear any pending IRQ flags
//  nRF24_ClearIRQFlags();
//  // Enable DPL
//  nRF24_SetDynamicPayloadLength(nRF24_DPL_ON);
//  nRF24_SetPayloadWithAck(1);
//  // Wake the transceiver
//  nRF24_SetPowerMode(nRF24_PWR_UP);
//  

        
        
  
  vSetStartADC();
  payload_length = 12;
  HAL_Delay(300); 
}

void LOGIC(){
  //чтение всех входов и выходов
  vReadStatePins();
  
  if(timer_SendState > 200){
    timer_SendState = 0;
    //периодическая отправка данных по NRF
    vSendStateJ();
    //контроль нажатия кнопки home, для индикации заряда
    if(jButton.bit.home){
      if(jMode.isMode == 0){//основной
        countIndZaradAkbJ++;
        if(countIndZaradAkbJ > 10)indZaradAkbJ = 1;//4сек
      }else{//дополнительный
        countIndZaradAkbRC++;
        if(countIndZaradAkbRC > 10)indZaradAkbRC = 1;//4сек
      }
    }else{ 
      countIndZaradAkbJ = 0;
      countIndZaradAkbRC = 0;
    }
  }
  
  //вся индикация
  if(indZaradAkbJ){
    vVisionLedBar(1, voltageAkbADC);
  }else if(indZaradAkbRC){
    vVisionLedBar(2, voltageAkbADC);
  }else{
    //навигация между режимами
    #ifdef MODE_ON
    vNavigationMode();
    #endif
    //индикация работы 4го светодиода 
    if(stateNrfTX == nRF24_TX_SUCCESS){
      vToogleLed4();
    }else{
      HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_15);
      Delay_ms(50);
    }
  }
  
  //режим сна
  if(timer_Sleep > TIMER_SLEEP){
    timer_Sleep=0;
    //индикация 4ой лампочки
    vSetStateGpio(0, GPIOC, GPIO_PIN_13);
    vSetStateGpio(0, GPIOC, GPIO_PIN_14);
    vSetStateGpio(0, GPIOC, GPIO_PIN_15);
    vSetStateGpio(1, GPIOA, GPIO_PIN_15);
    //остановка NRF
    nRF24_SetPowerMode(nRF24_PWR_DOWN);	
    //остановка тактирования
    HAL_TIM_Base_Stop_IT(&htim1);
    HAL_TIM_Base_Stop(&htim1);
    HAL_SuspendTick();//регистр  TICKINT
    //засыпаем
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON , PWR_SLEEPENTRY_WFI);
    //просыпаемся
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    //!!!здесь проще запустить перезагрузку МК, чем запуск всей переферии, так как сбрасываются все регисторы настроек при глубоком сне
    //сделанно это просто - не вкючаем тактирование таймера1, там где сбрасывется сторожевой таймер, он и перезагрузит.
  }
  HAL_Delay(3);
}

//чтение всех входов
void vReadStatePins(){
  static int temp=0;
  //светодиоды
  vSetStateGpio(jLed.one, GPIOC, GPIO_PIN_13);
  vSetStateGpio(jLed.two, GPIOC, GPIO_PIN_14);
  vSetStateGpio(jLed.fhree, GPIOC, GPIO_PIN_15);
  vSetStateGpio(jLed.four, GPIOA, GPIO_PIN_15);
  //кнопки (обрабатываем только те которые не EXTI)
  jButton.bit.up        = xGetStateGpio(jButton.bit.up,         GPIOB, GPIO_PIN_12);
  jButton.bit.dawn      = xGetStateGpio(jButton.bit.dawn,       GPIOB, GPIO_PIN_13);
  jButton.bit.write     = xGetStateGpio(jButton.bit.write,      GPIOB, GPIO_PIN_14);
  jButton.bit.left      = xGetStateGpio(jButton.bit.left,       GPIOB, GPIO_PIN_15);  
  jButton.bit.O         = xGetStateGpio(jButton.bit.O,          GPIOA, GPIO_PIN_8);
  jButton.bit.X         = xGetStateGpio(jButton.bit.X,          GPIOA, GPIO_PIN_9);
  jButton.bit.A         = xGetStateGpio(jButton.bit.A,          GPIOA, GPIO_PIN_10);
  jButton.bit.B         = xGetStateGpio(jButton.bit.B,          GPIOA, GPIO_PIN_11);
  jButton.bit.Lb        = xGetStateGpio(jButton.bit.Lb,         GPIOB, GPIO_PIN_3);
  jButton.bit.Ls        = xGetStateGpio(jButton.bit.Ls,         GPIOB, GPIO_PIN_4);
  jButton.bit.Wb        = xGetStateGpio(jButton.bit.Wb,         GPIOB, GPIO_PIN_5);
  jButton.bit.Ws        = xGetStateGpio(jButton.bit.Ws,         GPIOB, GPIO_PIN_6);
  //jButton.bit.start     = xGetStateGpio(jButton.bit.start,      GPIOB, GPIO_PIN_9);
  jButton.bit.select    = xGetStateGpio(jButton.bit.select,     GPIOB, GPIO_PIN_7);
  jButton.bit.home      = xGetStateGpio(jButton.bit.home,       GPIOB, GPIO_PIN_8);
  jButton.bit.back      = 0;//xGetStateGpio(jButton.bit.back,    GPIOA, GPIO_PIN_12);
  jButton.bit.list      = 0;//xGetStateGpio(jButton.bit.list,    GPIOB, GPIO_PIN_9);
  jButton.bit.StickA    = xGetStateGpio(jButton.bit.StickA,     GPIOB, GPIO_PIN_11);
  jButton.bit.StickB    = xGetStateGpio(jButton.bit.StickB,     GPIOB, GPIO_PIN_10);
  
  //кнопки которые в EXTI смотрим только отжатие
  if(jButton.bit.start) jButton.bit.start = xGetStateGpio(jButton.bit.start,   GPIOB, GPIO_PIN_9);
  
  //АЦП код на все каналы
  if(flagDmaAdc){
    flagDmaAdc = 0;
    HAL_ADC_Stop_DMA(&hadc1);
    
    //ADC_CHANNEL_0
    voltageAkbADC = (uint16_t)adc[0];
    
    //ADC_CHANNEL_1
    temp = (uint16_t)adc[1];
    temp -= jStickA.zeroValG;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickA.ValG = temp;
         timer_Sleep=0;
       }else jStickA.ValG = 0;
    
    //ADC_CHANNEL_2   
    temp = (uint16_t)adc[2];
    //приведение к нулю
    temp -= jStickA.zeroValV;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickA.ValV = temp;
         timer_Sleep=0;
       }else jStickA.ValV = 0;
    
    //ADC_CHANNEL_3
    temp = (uint16_t)adc[3];
    //приведение к нулю
    temp -= jStickB.zeroValG;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickB.ValG = temp;
         timer_Sleep=0;
       }else jStickB.ValG = 0;
    
    //ADC_CHANNEL_4
    temp = (uint16_t)adc[4];
    //приведение к нулю
    temp -= jStickB.zeroValV;
    //дергание на нуле
    if(temp > OFFSET_ZERO ||
       temp < -OFFSET_ZERO){
         jStickB.ValV = temp;
         timer_Sleep=0;
       }else jStickB.ValV = 0;
    
    adc[0] = 0;
    adc[1] = 0;
    adc[2] = 0;
    adc[3] = 0;
    adc[4] = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 5);
  };
}

//определение изменения состояния входа
uint8_t xGetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
  uint8_t state = stateInput;
  if(state != (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin))){
    state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    timer_Sleep=0;
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

//отправка данных
void vSendStateJ(){
  // подготовка данных
  jButton.bit.osn = jMode.osn;
  jButton.bit.dop = jMode.dop; 
  nRF24_payload[0] = jButton.uint8Message.gr1;
  nRF24_payload[1] = jButton.uint8Message.gr2;
  nRF24_payload[2] = jButton.uint8Message.gr3;
  nRF24_payload[3] = jButton.uint8Message.gr4;
  nRF24_payload[4] = jStickA.ValV;
  nRF24_payload[5] = jStickA.ValV >> 8;
  nRF24_payload[6] = jStickA.ValG;
  nRF24_payload[7] = jStickA.ValG >> 8;
  nRF24_payload[8] = jStickB.ValV;
  nRF24_payload[9] = jStickB.ValV >> 8;
  nRF24_payload[10] = jStickB.ValG;
  nRF24_payload[11] = jStickB.ValG >> 8;
  // отправка
  stateNrfTX = nRF24_TransmitPacket(nRF24_payload, payload_length);
  
//  otx = nRF24_GetRetransmitCounters();
//  nRF24_ReadPayloadDpl(nRF24_payload, &payload_length );
//  otx_plos_cnt = (otx & nRF24_MASK_PLOS_CNT) >> 4; // packets lost counter
//  otx_arc_cnt  = (otx & nRF24_MASK_ARC_CNT); // auto retransmissions counter
//  switch (stateNrfTX) {
//  case nRF24_TX_SUCCESS:
//    //UART_SendStr("OK");
//    break;
//  case nRF24_TX_TIMEOUT:
//    //UART_SendStr("TIMEOUT");
//    break;
//  case nRF24_TX_MAXRT:
//    //UART_SendStr("MAX RETRANSMIT");
//    packets_lost += otx_plos_cnt;
//    nRF24_ResetPLOS();
//    break;
//  default:
//    //UART_SendStr("ERROR");
//    break;
//  }
}

//стартовое измерение нуля
void vSetStartADC(){
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 5);
  HAL_Delay(200);
  if(flagDmaAdc){
    flagDmaAdc = 0;
    HAL_ADC_Stop_DMA(&hadc1);
    jStickA.zeroValV = (uint16_t)adc[1];
    jStickA.zeroValG = (uint16_t)adc[2];
    jStickB.zeroValV = (uint16_t)adc[3];
    jStickB.zeroValG = (uint16_t)adc[4];
    adc[1] = 0;
    adc[2] = 0;
    adc[3] = 0;
    adc[4] = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 5);
  };
}

//навигация между режимами
void vNavigationMode(){
  static uint8_t flag1, flag2;
  if(jButton.bit.select){ //перекл, осн/доп режим
    if(flag1){
      flag1 = 0;
      jMode.isMode = ~jMode.isMode & 0x01;
    }
  }else flag1 = 1;
  if(jButton.bit.start){ //перекл, режима(1,2,3)
    if(flag2){
      flag2 = 0;
      if(jMode.isMode == 0){
        switch(jMode.osn){
        case MODE_1:
          jMode.osn = MODE_2;
          break;
        case MODE_2:
          jMode.osn = MODE_3;
          break;
        case MODE_3:
          jMode.osn = MODE_1;
          break;
        }
      }else{
        switch(jMode.dop){
        case MODE_1:
          jMode.dop = MODE_2;
          break;
        case MODE_2:
          jMode.dop = MODE_3;
          break;
        case MODE_3:
          jMode.dop = MODE_1;
          break;
        }
      }
    }
  }else flag2 = 1;
  //индикация светодиодами
  if(jMode.isMode == 0){
    //режим
    switch(jMode.osn){
    case MODE_1:
      jLed.one   = 1;
      jLed.two   = 0;
      jLed.fhree = 0;
      break;
    case MODE_2:
      jLed.one   = 0;
      jLed.two   = 1;
      jLed.fhree = 0;
      break;
    case MODE_3:
      jLed.one   = 0;
      jLed.two   = 0;
      jLed.fhree = 1;
      break;
    }
  }else{
    //режим дополнительный
    switch(jMode.dop){
    case MODE_1:
      jLed.one   = vToogleLedLow(jLed.one);
      jLed.two   = 0;
      jLed.fhree = 0;
      break;
    case MODE_2:
      jLed.one   = 0;
      jLed.two   = vToogleLedLow(jLed.two);
      jLed.fhree = 0;
      break;
    case MODE_3:
      jLed.one   = 0;
      jLed.two   = 0;
      jLed.fhree = vToogleLedLow(jLed.fhree);
      break;
    }
  }
}

//мигание светодиода, медленное
uint8_t vToogleLedLow(uint8_t led){
  if(timer_LedLow > 600){
    timer_LedLow = 0;
    return ~led & 0x01;
  }else return led;
}

//мигание светодиода 4, режим норма
void vToogleLed4(){
  if(timer_Led4 > 3000){
    if(timer_Led4 > 3000 && timer_Led4 < 3010) jLed.four = 1;
    if(timer_Led4 > 3100){
      timer_Led4 = 0;
      jLed.four = 0;
    }
  }
}

//лимиты батареи
void vVisionLedBar(int mode, int value){
  if((value >= LIMIT_0BAR) && (value < LIMIT_1BAR)){
    vLedBar(mode,1);
  }
  else if((value >= LIMIT_1BAR) && (value < LIMIT_2BAR)){
    vLedBar(mode,2);
  }
  else if((value >= LIMIT_2BAR) && (value < LIMIT_3BAR)){
    vLedBar(mode,3);
  }
  else if((value >= LIMIT_3BAR) && (value < LIMIT_4BAR)){
    vLedBar(mode,4);
  }
}

//индикация состояния батареи
void vLedBar(int mode, int n){
  static int count = 0;
  static int flagMode = 1;
  switch (count){
  case 0:
    timer_ticBar = 0;
    count++;
    break;
  case 1:
    jLed.one   = 0;
    jLed.two   = 0;
    jLed.fhree = 0;
    jLed.four  = 0;
    if(timer_ticBar>300){
      jLed.one   = 1;
      jLed.two   = 1;
      jLed.fhree = 1;
      jLed.four  = 1;
      timer_ticBar = 0;
      count++;
    }
    break;
  case 2:
    if(timer_ticBar>250){
      jLed.one   = 0;
      jLed.two   = 0;
      jLed.fhree = 0;
      jLed.four  = 0;
      timer_ticBar = 0;
      count++;
      if(mode==2 && flagMode==1){count=1;flagMode=0;}
    }
    break;
  case 3:
    if(timer_ticBar>500){
      jLed.one   = 1;
      timer_ticBar = 0;
      flagMode=1;
      count++;
      if(n==1)count=7;
    }
    break;
  case 4:
    if(timer_ticBar>400){
      jLed.two   = 1;
      timer_ticBar = 0;
      count++;
      if(n==2)count=7;
    }
    break;
  case 5:
    if(timer_ticBar>400){
      jLed.fhree = 1;
      timer_ticBar = 0;
      count++;
      if(n==3)count=7;
    }
    break;
  case 6:
    if(timer_ticBar>400){
      jLed.four  = 1;
      timer_ticBar = 0;
      count++;
      if(n==4)count=7;
    }
    break;
  case 7:
    if(timer_ticBar>1900){
      jLed.one   = 0;
      jLed.two   = 0;
      jLed.fhree = 0;
      jLed.four  = 0;
      count=0;
      if(!jButton.bit.home){
        indZaradAkbJ = 0;
        indZaradAkbRC = 0;
        timer_Led4 = 3000;
      }
    }
    break;
  }
}

//масштабирование
int map_i (int x, int in_min, int in_max, int out_min, int out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

