///////////////////////////////////.h
//#define AVERAGING_ADC 10//���������� ����������
//��������� ������ ���
/*typedef struct{
  uint16_t Cannel_1;
  uint16_t Cannel_2;
  uint16_t Cannel_3;
  uint16_t Cannel_4;
}Cannel_t;*/

/*typedef struct{
  int element[AVERAGING_ADC];
}massADC_t;*/

///////////////////////////////////.c
//#include "stdio.h" //����� � �������//������� � ���� � View/Terminal I/O
/*
������� ��� ��� ���
  //ADC_CHANNEL_1-----------------------------
  //����������
  temp = xAveragingAdc(ADC_CHANNEL_1, &massAdcCanel1);
  //���������� � ����
  temp = temp - jStickA.zeroValV;
  //�������� �� ����
  if(temp > OFFSET_ZERO ||
     temp < -OFFSET_ZERO){
       jStickA.ValV.int16 = temp;
     }else jStickA.ValV.int16 = 0;

  //ADC_CHANNEL_2-----------------------------
  //����������
  temp = xAveragingAdc(ADC_CHANNEL_2, &massAdcCanel2);
  //���������� � ����
  temp = temp - jStickA.zeroValG;
  //�������� �� ����
  if(temp > OFFSET_ZERO ||
     temp < -OFFSET_ZERO){
       jStickA.ValG.int16 = temp;
     }else jStickA.ValG.int16 = 0;

  //ADC_CHANNEL_3-----------------------------
  //����������
  temp = xAveragingAdc(ADC_CHANNEL_3, &massAdcCanel3);
  //���������� � ����
  temp = temp - jStickB.zeroValV;
  //�������� �� ����
  if(temp > OFFSET_ZERO ||
     temp < -OFFSET_ZERO){
       jStickB.ValV.int16 = temp;
     }else jStickB.ValV.int16 = 0;

  //ADC_CHANNEL_4-----------------------------
  //����������
  temp = xAveragingAdc(ADC_CHANNEL_4, &massAdcCanel4);
  //���������� � ����
  temp = temp - jStickB.zeroValG;
  //�������� �� ����
  if(temp > OFFSET_ZERO ||
     temp < -OFFSET_ZERO){
       jStickB.ValG.int16 = temp;
     }else jStickB.ValG.int16 = 0;

  //����������� �������� ����
  //HAL_Delay(10);
*/

//����� ���
/*int xGetADCValue(uint32_t Channel){
  int val = 0;
  int Count = 3;//����������� 
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel=Channel;
  sConfig.Rank=ADC_REGULAR_RANK_1;
  sConfig.SamplingTime=ADC_SAMPLETIME_41CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc1,&sConfig);
  
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1,100000);
  
  for(int i = 0; i < Count; i++){
    val += HAL_ADC_GetValue(&hadc1);
  }
  HAL_ADC_Stop(&hadc1);
  return val / Count;
}*/
/*������� ��� ��� ���
  for(int i = 0; i < AVERAGING_ADC*10; i++){
    jStickA.zeroValV = xAveragingAdc(ADC_CHANNEL_1, &massAdcCanel1);
    jStickA.zeroValG = xAveragingAdc(ADC_CHANNEL_2, &massAdcCanel2);
    jStickB.zeroValV = xAveragingAdc(ADC_CHANNEL_3, &massAdcCanel3);
    jStickB.zeroValG = xAveragingAdc(ADC_CHANNEL_4, &massAdcCanel4);
  }*/
/* ����� �� ��������� �������� ���� ����������, ������� ��� ��� ���*/
//�-� ����������
/*int xAveragingAdc(uint32_t Channel, massADC_t* mass ){
  int result=0;
  for(int i = 0; i < AVERAGING_ADC; i++){
    if(i!=AVERAGING_ADC)mass->element[i] = mass->element[i+1];
    result += mass->element[i];
  }
  mass->element[AVERAGING_ADC-1] = (uint32_t) xGetADCValue(Channel);
  result /= AVERAGING_ADC;
  return result;
}*/


  //==== �������
  /*char str[12];
  sprintf(str, "%d  - %d %d %d %d"
		, jButton.uint32Message
                , jStickA.ValV.int16, jStickA.ValG.int16, jStickB.ValV.int16, jStickB.ValG.int16 );
  printf("%s\n",str);*/

  //�������� ��� ������ ���� ����
  /*sprintf(str, "%hx %hx %hx %hx - %hx %hx %hx %hx - %hx %hx %hx %hx"
		, nrf_data[0], nrf_data[1], nrf_data[2], nrf_data[3]
                , nrf_data[4], nrf_data[5], nrf_data[6], nrf_data[7]
                , nrf_data[8], nrf_data[9], nrf_data[10], nrf_data[11]);
  printf("%s\n",str);*/
  /*sprintf(str, "%d  - %d %d %d %d"
		, sizeof(jButton.uint32Message)
                , sizeof(jStickA.ValV), sizeof(jStickA.ValG), sizeof(jStickB.ValV), sizeof(jStickB.ValG) );
  printf("%s\n",str);*/
  /*sprintf(str, "%d"
		, sizeof(str));
  printf("%s\n",str);*/
  //==== //�������
////////////////////////////////////////////////////////////////////////////////
/*
������������ ��� ����� �������
isChipConnected();
NRF_Init();
//,,,,
setAutoAck(false);

disableDynamicPayloads();// o�������� ������������ �������� �������� �� ���� 

�������
disableCRC();//��������� CRC
//setRetries(2, 3);
setDataRate(RF24_1MBPS);
setPALevel(RF24_PA_LOW );//� ����� ��������� ���
setChannel(19); � ��������� �����. �� ��������� � NRF_Init() ��������������� 76. 

������������ �������� � 125. ������ ���� ���������� �� �������� � �� �����������.
setPayloadSize(3); � ������ �������� �������� � ������ ��� ����������� ������������ 

�������� ��������. ���������� ������ ���� ���������� �� �������� � �� �����������.

*/
