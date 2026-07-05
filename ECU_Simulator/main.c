#include "stm32f10x.h"
#include <stdio.h>

#define FILTER_DEPTH 8 

volatile uint16_t ADC_Values[2];
volatile uint8_t  flag_50ms    = 0;
volatile uint32_t msTicks      = 0;
volatile uint8_t  turn_left    = 0;
volatile uint8_t  turn_right   = 0;
volatile uint8_t  flag_turn    = 0;
volatile uint8_t  can_tx_status = 0;


uint16_t speed_buffer[FILTER_DEPTH] = {0};
uint16_t rpm_buffer[FILTER_DEPTH]   = {0};
uint8_t  filter_index = 0;

#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout; FILE __stdin; FILE __stderr;
void _sys_exit(int x) { x = x; }

int fputc(int ch, FILE *f) {
    USART_SendData(USART1, (uint8_t) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
    return ch;
}

void SysTick_Init(void) { if (SysTick_Config(SystemCoreClock / 1000)) { while (1); } }
void SysTick_Handler(void) { msTicks++; }

void Hardware_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO   | RCC_APB2Periph_USART1 |
                           RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* USART1 TX — PA9 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* USART1 RX — PA10 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void UART_Config(void) {
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate            = 115200; 
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void CAN_Config(void) {
    CAN_InitTypeDef       CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    CAN_InitStructure.CAN_TTCM = DISABLE;

    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;

    CAN_InitStructure.CAN_SJW        = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1        = CAN_BS1_14tq;
    CAN_InitStructure.CAN_BS2        = CAN_BS2_3tq;
    CAN_InitStructure.CAN_Prescaler  = 4; /* APB1=36MHz, 36/4/18TQ = 500kbps */


    uint8_t can_init_result = CAN_Init(CAN1, &CAN_InitStructure);
    if (can_init_result != CAN_InitStatus_Success) {
        printf("[CAN] Init FAILED (status=%d). Kiem tra SN65HVD230 va ket noi bus.\r\n",
               can_init_result);
    }

    /* B? l?c pass-all nh?n m?i ID */
    CAN_FilterInitStructure.CAN_FilterNumber         = 0;
    CAN_FilterInitStructure.CAN_FilterMode           = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale          = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh         = 0;
    CAN_FilterInitStructure.CAN_FilterIdLow          = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
}

void ADC_DMA_Config(void) {
    ADC_InitTypeDef  ADC_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* PA0, PA1 — Analog Input */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* DMA1 Channel1 */
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)ADC_Values;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = 2;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M               = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    /* ADC1 */
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode       = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel       = 2;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void TIMER3_Config(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;

    TIM_TimeBaseStructure.TIM_Prescaler   = 7200 - 1;
    TIM_TimeBaseStructure.TIM_Period      = 500  - 1; // 50ms
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM3, ENABLE);
}

void EXTI_Config(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);

    EXTI_InitStructure.EXTI_Line    = EXTI_Line12 | EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void CAN_Send_Speed(uint8_t speed_value) {
    CanTxMsg TxMessage;
    TxMessage.StdId    = 0x123;
    TxMessage.IDE      = CAN_ID_STD;
    TxMessage.RTR      = CAN_RTR_DATA;
    TxMessage.DLC      = 8;
    TxMessage.Data[0]  = speed_value;
    for (int i = 1; i < 8; i++) TxMessage.Data[i] = 0x00;
    can_tx_status = CAN_Transmit(CAN1, &TxMessage);
}

void CAN_Send_RPM(uint16_t rpm_value) {
    CanTxMsg TxMessage;
    TxMessage.StdId   = 0x124;
    TxMessage.IDE     = CAN_ID_STD;
    TxMessage.RTR     = CAN_RTR_DATA;
    TxMessage.DLC     = 8;
    TxMessage.Data[0] = (uint8_t)(rpm_value >> 8);
    TxMessage.Data[1] = (uint8_t)(rpm_value & 0xFF);
    for (int i = 2; i < 8; i++) TxMessage.Data[i] = 0x00;
    uint8_t s = CAN_Transmit(CAN1, &TxMessage);
    if (s == CAN_TxStatus_NoMailBox) can_tx_status = s;
}

void CAN_Send_TurnSignal(uint8_t left, uint8_t right) {
    CanTxMsg TxMessage;
    TxMessage.StdId   = 0x125;
    TxMessage.IDE     = CAN_ID_STD;
    TxMessage.RTR     = CAN_RTR_DATA;
    TxMessage.DLC     = 2;
    TxMessage.Data[0] = left;
    TxMessage.Data[1] = right;
    uint8_t s = CAN_Transmit(CAN1, &TxMessage);
    if (s == CAN_TxStatus_NoMailBox) can_tx_status = s;
}


uint16_t Update_Filter(uint16_t *buffer, uint16_t new_val) {
    uint32_t sum = 0;
    buffer[filter_index] = new_val;
    for (int i = 0; i < FILTER_DEPTH; i++) { sum += buffer[i]; }
    return (uint16_t)(sum / FILTER_DEPTH);
}

void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        flag_50ms = 1;
    }
}

void EXTI15_10_IRQHandler(void) {
    static uint32_t last_press_PB12 = 0;
    static uint32_t last_press_PB13 = 0;

    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line12);
        if (msTicks - last_press_PB12 >= 300) {
            last_press_PB12 = msTicks;
            turn_left  = 1;
            turn_right = 0;
            flag_turn  = 1;
   
        }
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line13);
        if (msTicks - last_press_PB13 >= 300) {
            last_press_PB13 = msTicks;
            turn_left  = 0;
            turn_right = 1;
            flag_turn  = 1;
     
        }
    }
}

int main(void) {
    uint8_t  real_speed    = 0;
    uint16_t real_rpm      = 0;
    uint8_t  print_counter = 0;
    uint16_t raw_speed     = 0;
    uint16_t raw_rpm       = 0;

    Hardware_GPIO_Config();
    UART_Config();
    SysTick_Init();
    CAN_Config();
    ADC_DMA_Config();
    EXTI_Config();
    TIMER3_Config();

    printf("[BOOT] STM32 IVI ECU started. CAN 500kbps PB8/PB9.\r\n");

    while (1) {
     
        if (flag_turn == 1) {
            __disable_irq();
            uint8_t local_left  = turn_left;
            uint8_t local_right = turn_right;
            flag_turn = 0;
            __enable_irq();
            
   
            CAN_Send_TurnSignal(local_left, local_right);
            printf("[EVENT] CAN 0x125: Left=%d, Right=%d\r\n", local_left, local_right);
        }

if (flag_50ms == 1) {
            flag_50ms = 0;

           
            uint16_t adc0 = ADC_Values[0];
            uint16_t adc1 = ADC_Values[1];

          
            float current_raw_speed = (float)((adc0 * 240UL) / 4095);
            float current_raw_rpm   = (float)((adc1 * 8000UL) / 4095);
            
       
            static float ema_speed = 0.0f;
            static float ema_rpm   = 1000.0f;
            const float alpha_speed = 0.20f; 
            const float alpha_rpm   = 0.25f; 
            
            ema_speed = (alpha_speed * current_raw_speed) + ((1.0f - alpha_speed) * ema_speed);
            ema_rpm   = (alpha_rpm * current_raw_rpm) + ((1.0f - alpha_rpm) * ema_rpm);

            // Ép v? ki?u s? nguyên
            uint8_t next_speed = (uint8_t)ema_speed;
            uint16_t next_rpm = (uint16_t)ema_rpm;


            if (abs(next_speed - real_speed) >= 1) {
                real_speed = next_speed;
            }
            if (abs(next_rpm - real_rpm) >= 15) { 
                real_rpm = next_rpm;
            }

         
            CAN_Send_Speed(real_speed);
            CAN_Send_RPM(real_rpm);

            print_counter++;
            if (print_counter >= 10) {
                print_counter = 0;
                printf("[SENSOR] Speed: %3d km/h | RPM: %4d | CAN Status: %d\r\n",
                       real_speed, real_rpm, can_tx_status);
            }
        }
    }
}