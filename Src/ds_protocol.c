/**
  ******************************************************************************
  * File Name          : ds_protocol.c
  * Description        : 
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
    /* Includes ------------------------------------------------------------------*/
#include "ds_protocol.h"


    uint8_t AckCmdBuffer[6];
    uint8_t CoreBoardCmdBuffer[DS_CMD_LEN + DS_DATA_LEN];
    uint8_t DoorBoardCmdBuffer[DS_CMD_LEN + DS_DATA_LEN];
    
    USARTRECIVETYPE     CoreBoardUsartType;
    USARTRECIVETYPE     DoorBoardUsartType;
    
    extern PROTOCOLCMD  gCoreBoardProtocolCmd;
    extern PROTOCOLCMD  gDoorBoardProtocolCmd;
    
    /*******************************************************************************
    *
    *       Function        :DS_CoreBoardUsartReceive_IDLE()
    *
    *       Input           :UART_HandleTypeDef
    *
    *       Return          :void
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    void DS_CoreBoardUsartReceive_IDLE(UART_HandleTypeDef *huart)
    {
      uint32_t  temp;
      if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))
      {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        HAL_UART_DMAStop(&huart1);
        temp = huart1.hdmarx->Instance->CNDTR;
        CoreBoardUsartType.RX_Size = DS_RX_LEN - temp;
        CoreBoardUsartType.RX_Flag = 1;
        HAL_UART_Receive_DMA(&huart1,CoreBoardUsartType.RX_pData,DS_RX_LEN);
      }
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_DoorBoardUsartReceive_IDLE()
    *
    *       Input           :UART_HandleTypeDef
    *
    *       Return          :void
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    void DS_DoorBoardUsartReceive_IDLE(UART_HandleTypeDef *huart)
    {
      uint32_t  temp;
      if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))
      {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);
        HAL_UART_DMAStop(&huart2);
        temp = huart2.hdmarx->Instance->CNDTR;
        DoorBoardUsartType.RX_Size = DS_RX_LEN - temp;
        DoorBoardUsartType.RX_Flag = 1;
        HAL_UART_Receive_DMA(&huart2,DoorBoardUsartType.RX_pData,DS_RX_LEN);    
      }
    }
    
    
    /*******************************************************************************
    *
    *       Function        :DS_CoreBoardProtocolInit()
    *
    *       Input           :void
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_CoreBoardProtocolInit(void)
    {
      DS_StatusTypeDef state = DS_OK;
      __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
      HAL_UART_Receive_DMA(&huart1,CoreBoardUsartType.RX_pData,DS_RX_LEN);
      
      CoreBoardCmdBuffer[0] = 0x5B;
      AckCmdBuffer[0] = 0x5B;
      AckCmdBuffer[5] = 0x5D;
      
      
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_DoorBoardProtocolInit()
    *
    *       Input           :void
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_DoorBoardProtocolInit(void)
    {
      DS_StatusTypeDef state = DS_OK;
      __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
      HAL_UART_Receive_DMA(&huart2,DoorBoardUsartType.RX_pData,DS_RX_LEN);
      
      DoorBoardCmdBuffer[0] = 0x5B;
      AckCmdBuffer[0] = 0x5B;
      AckCmdBuffer[5] = 0x5D;      
      
      HAL_GPIO_WritePin(CTR485_EN_GPIO_Port,CTR485_EN_Pin,GPIO_PIN_RESET);
      
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_SendDataToCoreBoard()
    *
    *       Input           :uint8_t *pData, uint16_t size,uint32_t Timeout
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_SendDataToCoreBoard(uint8_t *pData, uint16_t size,uint32_t Timeout)
    {
      DS_StatusTypeDef state = DS_OK;
      state = (DS_StatusTypeDef)HAL_UART_Transmit_DMA(&huart1, pData,size);
      return state;
    }
    
        /*******************************************************************************
    *
    *       Function        :DS_SendDataToDoorBoard()
    *
    *       Input           :uint8_t *pData, uint16_t size,uint32_t Timeout
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_SendDataToDoorBoard(uint8_t *pData, uint16_t size,uint32_t Timeout)
    {
      DS_StatusTypeDef state = DS_OK;
      state = (DS_StatusTypeDef)HAL_UART_Transmit_DMA(&huart2, pData,size);
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_HandingUartDataFromCoreBoard()
    *
    *       Input           :void
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/25
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_HandingUartDataFromCoreBoard(void)
    {
      DS_StatusTypeDef state = DS_OK;
      uint16_t i;
      if(!CoreBoardUsartType.RX_Flag)
      {
        return state;
      }
      CoreBoardUsartType.RX_Flag = 0;
      if(gCoreBoardProtocolCmd.RevOrSendFlag)
      {
        return state;
      }
      if(gCoreBoardProtocolCmd.RevRequestFlag)
      {
        return state;
      }
      
      return state;
    }




   
  /**
  * @}
  */
  /**
  * @}
  */
  /*****************************END OF FILE**************************************/
