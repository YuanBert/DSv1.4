/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "ds_led.h"
#include "ds_protocol.h"
#include "ds_gentlesensor.h"
#include "ds_log.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern  USARTRECIVETYPE     CoreBoardUsartType;
extern  USARTRECIVETYPE     DoorBoardUsartType;

uint8_t gSendDoorBoardALogFlag;
uint8_t DoorBoardALogBuffer[31];

PROTOCOLCMD  gCoreBoardProtocolCmd;
PROTOCOLCMD  gDoorBoardProtocolCmd;
GPIOSTATUSDETECTION gGentleSensorStatusDetection;
GPIOSTATUSDETECTION gMCUKeyInGpio;

uint32_t ADC_Value[10];
uint32_t gLightADCValue;
uint32_t gTempValue;

uint8_t gSetBuffer[20];
uint8_t gSendLogReportFlag = 1;
uint8_t gSendLogFlag = 0;
uint8_t gSendOpenFlag = 0;
uint8_t gLEDsCarFlag = 0;

uint32_t    gLogCnt;
uint16_t    gTIM4Cnt;
uint8_t     gTIM4CntFlag;

uint16_t    gTIM5Cnt;
uint8_t     gTIM5CntFlag;
uint16_t    gTIM5LedCnt;
uint8_t     gTIM5LedFlag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t i = 0;
  gMCUKeyInGpio.GpioSendDataFlag = 0;
  gMCUKeyInGpio.GpioFilterCntSum = 5;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();

  /* USER CODE BEGIN 2 */
  DS_LED_Init();
  
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&ADC_Value,10);
  
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim5);
  
  DS_GentleSensorInit();
  DS_CoreBoardProtocolInit();
  DS_DoorBoardProtocolInit();
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim5);
  
  DS_RUNNINGLED_ON();
  
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    if(gTIM5LedFlag)
    {
      DS_LEDS_TEST();
      gTIM5LedFlag = 0;
    }
    
    if(gLogCnt > 10000)
    {
      DS_ReportLogInfo();
      gLogCnt= 0;
    }
    
    if(gSendLogFlag && gSendLogReportFlag)
    {
      DS_ReportLogInfo();
      gSendLogFlag = 0;
    }
    
    /* ��Ƶ�������� */
    if(gMCUKeyInGpio.GpioSendDataFlag)
    {
      DS_SendHelpCmd();
      gMCUKeyInGpio.GpioSendDataFlag = 0;
    }
    
    DS_GentleSensorCheck();
    DS_HandingUartDataFromCoreBoard();
    DS_HandingCmdFromCoreBoard(&gCoreBoardProtocolCmd);
    
    DS_HandingUartDataFromDoorBoard();
    DS_HandingCmdFromDoorBoard(&gDoorBoardProtocolCmd);
    
    /* �ϴ���բA��Ϣ */
    if(2 == gSendDoorBoardALogFlag)
    {
      DS_SendDataToCoreBoard(DoorBoardALogBuffer,31,0xFFFF);
      for(i = 0; i < 31; i++)
      {
        DoorBoardALogBuffer[i] = 0;
      }
      i = 0;
      gSendDoorBoardALogFlag = 0;
    }
    
    if(1 == gTIM5CntFlag)
    {
      DS_TrySend5TimesCmdToCoreBoard(&gCoreBoardProtocolCmd);
      DS_TrySend5TimesCmdToDoorBoard(&gDoorBoardProtocolCmd);
      for(i = 0; i < 10;)
      {
        gLightADCValue += ADC_Value[i++];
        gTempValue += ADC_Value[i++];
      }
      gLightADCValue /= 5;
      gTempValue /=5;
      
      DS_UpTemInfoLog(gTempValue);
      DS_UpLightInfoLog(gLightADCValue);
      
      gLightADCValue = 0;
      gTempValue = 0;
      gTIM5CntFlag = 0;
    }
    
    if(gSendOpenFlag)
    {
      DS_SendOPenDoorCmd();
      gSendOpenFlag = 0;
    }
    
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** NVIC Configuration
*/
static void MX_NVIC_Init(void)
{
  /* TIM4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* TIM5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM5_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
  /* ADC1_2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the __HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
  /* 0.1 ms*/
  if(htim4.Instance == htim->Instance)
  {
    gGentleSensorStatusDetection.GpioCurrentReadVal = HAL_GPIO_ReadPin(GentleSensor_GPIO_Port,GentleSensor_Pin);
    if(0 == gGentleSensorStatusDetection.GpioCurrentReadVal && 0 == gGentleSensorStatusDetection.GpioLastReadVal)
    {
      if(0 == gGentleSensorStatusDetection.GpioCheckedFlag)
      {
        gGentleSensorStatusDetection.GpioFilterCnt ++;
        if(gGentleSensorStatusDetection.GpioFilterCnt > gGentleSensorStatusDetection.GpioFilterCntSum && 0 == gGentleSensorStatusDetection.GpioStatusVal)
        {
          DS_UpgentleStatusInfoLog(1);
          gGentleSensorStatusDetection.GpioStatusVal = 1;
          gGentleSensorStatusDetection.GpioFilterCnt = 0;
          gGentleSensorStatusDetection.GpioCheckedFlag = 1;
        }
      }
    }
    else
    {
      DS_UpgentleStatusInfoLog(0);
      gGentleSensorStatusDetection.GpioCheckedFlag       = 0;
      gGentleSensorStatusDetection.GpioFilterCnt     = 0;
      gGentleSensorStatusDetection.GpioStatusVal     = 0;
      gGentleSensorStatusDetection.GpioSendDataFlag  = 1;
    }     
    gGentleSensorStatusDetection.GpioLastReadVal = gGentleSensorStatusDetection.GpioCurrentReadVal; 
  }
  
  /* 1ms */
  if(htim5.Instance == htim->Instance)
  {
    gLogCnt ++;
    gTIM5LedCnt++;
    if(gTIM5LedCnt > 200)
    {
      gTIM5LedFlag = 1;
      gTIM5LedCnt = 0;
    }

    gTIM5Cnt++;
    if(gTIM5Cnt > 300)
    {
      gTIM5CntFlag = 1;
      gTIM5Cnt = 0;
    }   
    
    if(gGentleSensorStatusDetection.GpioValidLogicTimeCnt > 80)
    {
      gGentleSensorStatusDetection.GpioValidLogicTimeCnt--;
    }
    
    gMCUKeyInGpio.GpioCurrentReadVal = HAL_GPIO_ReadPin(MCU_KEY_IN_GPIO_Port,MCU_KEY_IN_Pin);
    if(0 == gMCUKeyInGpio.GpioCurrentReadVal && 0 == gMCUKeyInGpio.GpioLastReadVal)
    {
      if(0 == gMCUKeyInGpio.GpioCheckedFlag)
      {
        gMCUKeyInGpio.GpioFilterCnt ++;
        if(gMCUKeyInGpio.GpioFilterCnt > gMCUKeyInGpio.GpioFilterCntSum && gMCUKeyInGpio.GpioStatusVal)
        {
          gMCUKeyInGpio.GpioStatusVal = 1;
          gMCUKeyInGpio.GpioFilterCnt = 0;
          gMCUKeyInGpio.GpioCheckedFlag = 1;
          gMCUKeyInGpio.GpioSendDataFlag = 1;
        }
      }
    }
    else
    {
      gMCUKeyInGpio.GpioStatusVal = 0;
      gMCUKeyInGpio.GpioFilterCnt = 0;
      gMCUKeyInGpio.GpioCheckedFlag = 0;      
    }
    gMCUKeyInGpio.GpioLastReadVal = gMCUKeyInGpio.GpioCurrentReadVal;
  }

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
