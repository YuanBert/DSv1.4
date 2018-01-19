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
#include "ds_log.h"
#include "ds_gentlesensor.h"
    
    uint8_t AckCmdBuffer[6];
    uint8_t CoreBoardCmdBuffer[DS_CMD_LEN + DS_DATA_LEN];
    uint8_t DoorBoardCmdBuffer[DS_CMD_LEN + DS_DATA_LEN];
    
    USARTRECIVETYPE     CoreBoardUsartType;
    USARTRECIVETYPE     DoorBoardUsartType;
    
    extern uint8_t gSendLogReportFlag;
    
    extern uint8_t gSendDoorBoardALogFlag;
    extern uint8_t DoorBoardALogBuffer[31];
      
    extern GPIOSTATUSDETECTION gGentleSensorStatusDetection;
    extern PROTOCOLCMD  gCoreBoardProtocolCmd;
    extern PROTOCOLCMD  gDoorBoardProtocolCmd;
    
    extern uint8_t gSendOpenFlag;
    extern uint8_t gSendLogFlag;
    static uint8_t getXORCode(uint8_t* pData,uint16_t len)
    {
      uint8_t ret;
      uint16_t i;
      ret = pData[0];
      for(i = 1; i < len; i++)
      {
        ret ^=pData[i];
      }
      return ret;
    }
    
   DS_StatusTypeDef DS_SendOPenDoorCmd(void)
    {
      DS_StatusTypeDef state = DS_OK;
      gDoorBoardProtocolCmd.CmdParam = 0x01;
      gDoorBoardProtocolCmd.CmdType = 0XB2;
      gDoorBoardProtocolCmd.DataLengthHight = 0;
      gDoorBoardProtocolCmd.DataLengthLow = 0;
      gDoorBoardProtocolCmd.DataLength = 0;
      state = DS_SendRequestCmdToDoorBoard(&gDoorBoardProtocolCmd);
      gCoreBoardProtocolCmd.AckCodeL = state;
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_SendRequestCmd()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd,uint8_t *pCmdDataBuffer
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    static DS_StatusTypeDef DS_SendRequestCmd(pPROTOCOLCMD pRequestCmd,uint8_t *pCmdDataBuffer)
    {
      uint16_t dataLength = 0;
      uint8_t   tempXOR = 0;
      DS_StatusTypeDef state = DS_OK;
      if(0 == pRequestCmd->HandingFlag)
      {
        pRequestCmd->HandingFlag = 1;
        *(pCmdDataBuffer + 1) = pRequestCmd->CmdType;
        *(pCmdDataBuffer + 2)= pRequestCmd->CmdParam;
        *(pCmdDataBuffer + 3)= pRequestCmd->DataLengthHight;
        *(pCmdDataBuffer + 4)= pRequestCmd->DataLengthLow;
        dataLength = pRequestCmd->DataLength;
        *(pCmdDataBuffer + REQUESTFIXEDCOMMANDLEN + dataLength - 1) = 0x5D;
        pRequestCmd->TotalLength = dataLength + REQUESTFIXEDCOMMANDLEN;
        
        /* Calculate XOR */
        tempXOR = getXORCode(pCmdDataBuffer + 1, pRequestCmd->TotalLength - 3);
        
        *(pCmdDataBuffer + dataLength + REQUESTFIXEDCOMMANDLEN - 2) = (uint8_t)tempXOR;
        
        
        pRequestCmd->RevOrSendFlag = 1;
        pRequestCmd->RevEchoFlag = 0;
      }
      return state;   
    }

     
    
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
      state = (DS_StatusTypeDef)HAL_UART_Transmit(&huart1, pData,size,0xFFFF);
      if(DS_OK != state)
      {
        state = DS_ERROR;
      }
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
      HAL_GPIO_WritePin(CTR485_EN_GPIO_Port,CTR485_EN_Pin,GPIO_PIN_SET);
      state = (DS_StatusTypeDef)HAL_UART_Transmit(&huart2, pData,size,Timeout);
      HAL_GPIO_WritePin(CTR485_EN_GPIO_Port,CTR485_EN_Pin,GPIO_PIN_RESET);
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
      uint8_t xorTemp;
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
      
      if(gCoreBoardProtocolCmd.RevDataCnt < gCoreBoardProtocolCmd.DataLength && 0 != gCoreBoardProtocolCmd.RevDataCnt)
      {
        for(i = 0; i < CoreBoardUsartType.RX_Size; i++)
        {
          CoreBoardCmdBuffer[5 + gCoreBoardProtocolCmd.RevDataCnt + i] = *(CoreBoardUsartType.RX_pData + i);
          gCoreBoardProtocolCmd.RevDataCnt ++;
          if(gCoreBoardProtocolCmd.DataLength == gCoreBoardProtocolCmd.RevDataCnt)
          {
            gCoreBoardProtocolCmd.XOR8BIT     =*(CoreBoardUsartType.RX_pData + i + 1);
            if(0x5D != *(CoreBoardUsartType.RX_pData + i + 2))
            {
              gCoreBoardProtocolCmd.RevDataCnt          = 0;
              gCoreBoardProtocolCmd.DataLength          = 0;
              gCoreBoardProtocolCmd.TotalLength         = 0;
              return state;
            }
            gCoreBoardProtocolCmd.TotalLength   = gCoreBoardProtocolCmd.DataLength + REQUESTFIXEDCOMMANDLEN;
            /* here to check XOR code */
            xorTemp = getXORCode(CoreBoardCmdBuffer + 1,gCoreBoardProtocolCmd.TotalLength - 3);
            if(gCoreBoardProtocolCmd.XOR8BIT != xorTemp)
            {
              gCoreBoardProtocolCmd.TotalLength = 0;
              return state;
            }
            
            gCoreBoardProtocolCmd.RevRequestFlag = 1;
            return state;
          }
        }
        return state;
      }
      
      if(0 == gCoreBoardProtocolCmd.TotalLength)
      {
        if(0x5B != *(CoreBoardUsartType.RX_pData))
        {
          return state;
        }
        
        if(0xA0 == (*(CoreBoardUsartType.RX_pData + 1) & 0xF0) && 0x5D == *(CoreBoardUsartType.RX_pData + ACKFIXEDCOMMANDLEN - 1))
        {
          gCoreBoardProtocolCmd.AckCmdCode           =*(CoreBoardUsartType.RX_pData + 1);
          gCoreBoardProtocolCmd.AckCodeH             =*(CoreBoardUsartType.RX_pData + 2);
          gCoreBoardProtocolCmd.AckCodeL             =*(CoreBoardUsartType.RX_pData + 3);
          gCoreBoardProtocolCmd.AckXOR8BIT           =*(CoreBoardUsartType.RX_pData + 4);
          
          /* Here to add XOR code */
          xorTemp = getXORCode(CoreBoardUsartType.RX_pData + 1, 3);
          if(gCoreBoardProtocolCmd.AckXOR8BIT != xorTemp)
          {
            return state;
          }
          gCoreBoardProtocolCmd.RevEchoFlag = 1;
          
          return state;
        }
        
        gCoreBoardProtocolCmd.CmdType           =*(CoreBoardUsartType.RX_pData + 1);
        gCoreBoardProtocolCmd.CmdParam          =*(CoreBoardUsartType.RX_pData + 2);
        
        gCoreBoardProtocolCmd.DataLengthLow     =*(CoreBoardUsartType.RX_pData + 4);
        gCoreBoardProtocolCmd.DataLengthHight   =*(CoreBoardUsartType.RX_pData + 3);
        
        CoreBoardCmdBuffer[1]   = gCoreBoardProtocolCmd.CmdType;
        CoreBoardCmdBuffer[2]   = gCoreBoardProtocolCmd.CmdParam;
        CoreBoardCmdBuffer[3]   = gCoreBoardProtocolCmd.DataLengthHight;
        CoreBoardCmdBuffer[4]   = gCoreBoardProtocolCmd.DataLengthLow;
        
        gCoreBoardProtocolCmd.DataLength = (gCoreBoardProtocolCmd.DataLengthHight << 8) + gCoreBoardProtocolCmd.DataLengthLow;
        
        
        if(0 == gCoreBoardProtocolCmd.DataLength)
        {
          if(0x5D != *(CoreBoardUsartType.RX_pData + REQUESTFIXEDCOMMANDLEN - 1))
          {
            return state;
          }
          gCoreBoardProtocolCmd.XOR8BIT         =*(CoreBoardUsartType.RX_pData + 5);
          gCoreBoardProtocolCmd.TotalLength     = REQUESTFIXEDCOMMANDLEN;
          /* here to check XOR */
          xorTemp = getXORCode(CoreBoardCmdBuffer + 1, gCoreBoardProtocolCmd.TotalLength - 3);
          if(gCoreBoardProtocolCmd.XOR8BIT != xorTemp)
          {
            gCoreBoardProtocolCmd.TotalLength   = 0;
            return state;
          }
          gCoreBoardProtocolCmd.RevRequestFlag  = 1;
          
          return state;
        }
        
        for(i = 5; i < CoreBoardUsartType.RX_Size; i++)
        {
          CoreBoardCmdBuffer[i] = *(CoreBoardUsartType.RX_pData + i);
          gCoreBoardProtocolCmd.RevDataCnt ++;
          if(gCoreBoardProtocolCmd.DataLength == gCoreBoardProtocolCmd.RevDataCnt)
          {
            if(0x5D != *(CoreBoardUsartType.RX_pData + REQUESTFIXEDCOMMANDLEN + gCoreBoardProtocolCmd.RevDataCnt -1))
            {
              gCoreBoardProtocolCmd.RevDataCnt          = 0;
              gCoreBoardProtocolCmd.DataLength          = 0;
              gCoreBoardProtocolCmd.TotalLength         = 0;
              return state;
            }
            gCoreBoardProtocolCmd.XOR8BIT     = *(CoreBoardUsartType.RX_pData + i + 1);
            gCoreBoardProtocolCmd.TotalLength = REQUESTFIXEDCOMMANDLEN + gCoreBoardProtocolCmd.DataLength;
            /* here add to check XOR */
            xorTemp = getXORCode(CoreBoardCmdBuffer + 1,gCoreBoardProtocolCmd.TotalLength - 3);
            if(gCoreBoardProtocolCmd.XOR8BIT != xorTemp)
            {
              gCoreBoardProtocolCmd.TotalLength = 0;
              return state;
            }
            gCoreBoardProtocolCmd.RevRequestFlag = 1;
            return state;
          }
        } 
      }
      
      return state;
    }
    /*******************************************************************************
    *
    *       Function        :DS_HandingUartDataFromDoorBoard()
    *
    *       Input           :void
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_HandingUartDataFromDoorBoard(void)
    {
      DS_StatusTypeDef state = DS_OK;
      uint8_t xorTemp;
      uint16_t i;
      
      if(!DoorBoardUsartType.RX_Flag)
      {
        return state;
      }
      DoorBoardUsartType.RX_Flag = 0;
      if(gDoorBoardProtocolCmd.RevOrSendFlag)
      {
        return state;
      }
      if(gDoorBoardProtocolCmd.RevRequestFlag)
      {
        return state;
      }
      
      if(gDoorBoardProtocolCmd.RevDataCnt < gDoorBoardProtocolCmd.DataLength && 0 != gDoorBoardProtocolCmd.RevDataCnt)
      {
        for(i = 0; i < DoorBoardUsartType.RX_Size; i++)
        {
          DoorBoardCmdBuffer[5 + gDoorBoardProtocolCmd.RevDataCnt + i] = *(DoorBoardUsartType.RX_pData + i);
          gDoorBoardProtocolCmd.RevDataCnt ++;
          if(gDoorBoardProtocolCmd.DataLength == gDoorBoardProtocolCmd.RevDataCnt)
          {
            gDoorBoardProtocolCmd.XOR8BIT     =*(DoorBoardUsartType.RX_pData + i + 1);
            DoorBoardCmdBuffer[5 + gDoorBoardProtocolCmd.RevDataCnt + i] = gDoorBoardProtocolCmd.XOR8BIT;
            
            if(0x5D != *(DoorBoardUsartType.RX_pData + i + 2))
            {
              gDoorBoardProtocolCmd.RevDataCnt          = 0;
              gDoorBoardProtocolCmd.DataLength          = 0;
              gDoorBoardProtocolCmd.TotalLength         = 0;
              return state;
            }
            DoorBoardCmdBuffer[5 + gDoorBoardProtocolCmd.RevDataCnt + i + 1] = 0x5D;
            gDoorBoardProtocolCmd.TotalLength = gDoorBoardProtocolCmd.DataLength + REQUESTFIXEDCOMMANDLEN;
            /* here to check XOR code */
            xorTemp = getXORCode(DoorBoardCmdBuffer + 1, gDoorBoardProtocolCmd.TotalLength - 3);
            if(gDoorBoardProtocolCmd.XOR8BIT != xorTemp)
            {
              gDoorBoardProtocolCmd.TotalLength = 0;
              return state;
            }
            gDoorBoardProtocolCmd.RevRequestFlag = 1;
            return state;
          }
        }
        return state;
      }
      
      if(0 == gDoorBoardProtocolCmd.TotalLength)
      {
        if(0x5B != *(DoorBoardUsartType.RX_pData))
        {
          return state;
        }
        
        if(0xA0 == (*(DoorBoardUsartType.RX_pData + 1) & 0xF0) && 0x5D == *(DoorBoardUsartType.RX_pData + ACKFIXEDCOMMANDLEN - 1))
        {
          gDoorBoardProtocolCmd.AckCmdCode             =*(DoorBoardUsartType.RX_pData + 1);
          gDoorBoardProtocolCmd.AckCodeH               =*(DoorBoardUsartType.RX_pData + 2);
          gDoorBoardProtocolCmd.AckCodeL               =*(DoorBoardUsartType.RX_pData + 3);
          gDoorBoardProtocolCmd.AckXOR8BIT             =*(DoorBoardUsartType.RX_pData + 4);
          
          /* Here to add XOR code */
          xorTemp = getXORCode(DoorBoardUsartType.RX_pData + 1, 3);
          if(gDoorBoardProtocolCmd.AckXOR8BIT != xorTemp)
          {
            return state;
          }
          gDoorBoardProtocolCmd.RevEchoFlag = 1;
          return state;
        }
        
        gDoorBoardProtocolCmd.CmdType           =*(DoorBoardUsartType.RX_pData + 1);
        gDoorBoardProtocolCmd.CmdParam          =*(DoorBoardUsartType.RX_pData + 2);
        
        gDoorBoardProtocolCmd.DataLengthLow     =*(DoorBoardUsartType.RX_pData + 4);
        gDoorBoardProtocolCmd.DataLengthHight   =*(DoorBoardUsartType.RX_pData + 3);
        
        DoorBoardCmdBuffer[1]   = gDoorBoardProtocolCmd.CmdType;
        DoorBoardCmdBuffer[2]   = gDoorBoardProtocolCmd.CmdParam;
        DoorBoardCmdBuffer[3]   = gDoorBoardProtocolCmd.DataLengthHight;
        DoorBoardCmdBuffer[4]   = gDoorBoardProtocolCmd.DataLengthLow;
        
        gDoorBoardProtocolCmd.DataLength = (gDoorBoardProtocolCmd.DataLengthHight << 8) + gDoorBoardProtocolCmd.DataLengthLow;
        
        
        if(0 == gDoorBoardProtocolCmd.DataLength)
        {
          if(0x5D != *(DoorBoardUsartType.RX_pData + REQUESTFIXEDCOMMANDLEN - 1))
          {
            return state;
          }
          DoorBoardCmdBuffer[5] = *(DoorBoardUsartType.RX_pData + 5);
          DoorBoardCmdBuffer[6] = *(DoorBoardUsartType.RX_pData + REQUESTFIXEDCOMMANDLEN - 1);
          gDoorBoardProtocolCmd.XOR8BIT       =*(DoorBoardUsartType.RX_pData + 5);
          gDoorBoardProtocolCmd.TotalLength   = REQUESTFIXEDCOMMANDLEN;
          /* here to check XOR */
          xorTemp = getXORCode(DoorBoardCmdBuffer + 1,gDoorBoardProtocolCmd.TotalLength - 3);
          if(gDoorBoardProtocolCmd.XOR8BIT != xorTemp)
          {
            gDoorBoardProtocolCmd.TotalLength    = 0;
            return state;
          }
          gDoorBoardProtocolCmd.RevRequestFlag  = 1;
          return state;
        }
        
        for(i = 5; i < DoorBoardUsartType.RX_Size; i++)
        {
          DoorBoardCmdBuffer[i] = *(DoorBoardUsartType.RX_pData + i);
          gDoorBoardProtocolCmd.RevDataCnt ++;
          if(gDoorBoardProtocolCmd.DataLength == gDoorBoardProtocolCmd.RevDataCnt)
          {
            if(0x5D != *(DoorBoardUsartType.RX_pData + REQUESTFIXEDCOMMANDLEN + gDoorBoardProtocolCmd.RevDataCnt -1))
            {
              gDoorBoardProtocolCmd.RevDataCnt          = 0;
              gDoorBoardProtocolCmd.DataLength          = 0;
              gDoorBoardProtocolCmd.TotalLength         = 0;
              return state;
            }
            DoorBoardCmdBuffer[i + 2]            = 0x5D;
            DoorBoardCmdBuffer[i + 1]            = *(DoorBoardUsartType.RX_pData + i + 1);
            gDoorBoardProtocolCmd.XOR8BIT        = *(DoorBoardUsartType.RX_pData + i + 1);
            gDoorBoardProtocolCmd.TotalLength    = REQUESTFIXEDCOMMANDLEN + gDoorBoardProtocolCmd.DataLength;
            
            /* here add to check XOR */
            xorTemp = getXORCode(DoorBoardCmdBuffer + 1, gDoorBoardProtocolCmd.TotalLength - 3);
            if(gDoorBoardProtocolCmd.XOR8BIT != xorTemp)
            {
              gDoorBoardProtocolCmd.TotalLength = 0;
              return state;
            }
            gDoorBoardProtocolCmd.RevRequestFlag = 1;
            return state;
          }
        } 
      }      
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_SendRequestCmdToCoreBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_SendRequestCmdToCoreBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      if(pRequestCmd->RevRequestFlag)
      {
        return state;
      }
      state = DS_SendRequestCmd(pRequestCmd, CoreBoardCmdBuffer);
      state = DS_SendDataToCoreBoard(CoreBoardCmdBuffer,pRequestCmd->TotalLength,0xFFFF);
      
      return state;
    }
    /*******************************************************************************
    *
    *       Function        :DS_SendRequestCmdToDoorBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_SendRequestCmdToDoorBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
//      if(pRequestCmd->RevRequestFlag)
//      {
//        return state;
//      }
      state = DS_SendRequestCmd(pRequestCmd, DoorBoardCmdBuffer);  
      state = DS_SendDataToDoorBoard(DoorBoardCmdBuffer,pRequestCmd->TotalLength,0xFFFF);
      
      return state;
      
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_AckRequestCmdFromCoreBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_AckRequestCmdFromCoreBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      AckCmdBuffer[1] = pRequestCmd->AckCmdCode;
      AckCmdBuffer[2] = pRequestCmd->AckCodeH;
      AckCmdBuffer[3] = pRequestCmd->AckCodeL;
      AckCmdBuffer[4] = getXORCode(AckCmdBuffer + 1, 3);
      
      state = DS_SendDataToCoreBoard(AckCmdBuffer,6,0xFFFF);  
      return state;
    }
    
        /*******************************************************************************
    *
    *       Function        :DS_AckRequestCmdFromCoreBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_AckRequestCmdFromDoorBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      
      AckCmdBuffer[1] = pRequestCmd->AckCmdCode;
      AckCmdBuffer[2] = pRequestCmd->AckCodeH;
      AckCmdBuffer[3] = pRequestCmd->AckCodeL;
      AckCmdBuffer[4] = getXORCode(AckCmdBuffer + 1, 3);
      
      state = DS_SendDataToDoorBoard(AckCmdBuffer,6,0xFFFF);  
      return state;
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_HandingCmdFromCoreBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_HandingCmdFromCoreBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      if(1 == pRequestCmd->RevRequestFlag)
      {
        switch((pRequestCmd->CmdType) & 0xF0)
        {
        case 0xB0: pRequestCmd->AckCmdCode = 0xAB;
                   if(0xB2 == pRequestCmd->CmdType && 0x01 == pRequestCmd->CmdParam)
                   {
                     pRequestCmd->AckCodeH   = 0x02;
                     if(gGentleSensorStatusDetection.GpioStatusVal) //车未离场，则发送开闸指令，并返回开闸结果
                     {
                        pRequestCmd->AckCodeL   = 0x00;
                        gSendOpenFlag = 1;
                     }
                     else
                     {
                       gSendOpenFlag = 1;  
                       pRequestCmd->AckCodeL = 0x01;
                     }
                   }
                   break;
                   
        case 0xC0: pRequestCmd->AckCmdCode = 0xAC;
                   if(0xC1 == pRequestCmd->CmdType)
                   {
                      pRequestCmd->AckCodeH = 0x01;
                      pRequestCmd->AckCodeL = pRequestCmd->CmdParam;
                      break;
                   }
                   if(0xC2 == pRequestCmd->CmdType)
                   {
                      pRequestCmd->AckCodeH = 0x02;
                      pRequestCmd->AckCodeL = pRequestCmd->CmdParam;
                      break;
                   }
                   if(0xC3 == pRequestCmd->CmdType)
                   {
                     pRequestCmd->AckCodeH = 0x03;
                     pRequestCmd->AckCodeL = pRequestCmd->CmdParam;
                     break;
                   }
                     
                   break;
        case 0xD0: pRequestCmd->AckCmdCode = 0xAD;
                   if(0xD1 == pRequestCmd->CmdType)
                   {
                     pRequestCmd->AckCodeH = 0x01; 
                     gSendLogFlag = 1;
                     //DS_ReportLogInfo();
                     break;
                   }
                   if(0xD0 == pRequestCmd->CmdType)
                   {
                     pRequestCmd->AckCodeH = 0x00;
                     if(0x01 == pRequestCmd->CmdParam)
                     {
                        gSendLogReportFlag = 1;
                        break;
                     }
                     
                     if(0x02 == pRequestCmd->CmdParam)
                     {
                        gSendLogReportFlag = 0;
                        break;
                     }
                   }
                   break;
        case 0xE0: pRequestCmd->AckCmdCode = 0xAE;
                   break;
        case 0xF0: pRequestCmd->AckCmdCode = 0xAF;
                   break;
        
        
        default: state = DS_NOCMD; break;
        
        }
        
        DS_AckRequestCmdFromCoreBoard(pRequestCmd);
        
//        if(gSendLogFlag)
//        {
//          HAL_Delay(1);
//          DS_ReportLogInfo();
//          gSendLogFlag = 0;
//        }
        
        pRequestCmd->HandingFlag      = 0;
        pRequestCmd->RevRequestFlag   = 0;
        pRequestCmd->DataLength       = 0;
        pRequestCmd->DataLengthHight  = 0;
        pRequestCmd->DataLengthLow    = 0;
        pRequestCmd->RevDataCnt       = 0;
        pRequestCmd->TotalLength      = 0;
      }
      
      return state;
    }
    /*******************************************************************************
    *
    *       Function        :DS_HandingCmdFromDoorBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_HandingCmdFromDoorBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      uint8_t i = 0;
      if(1 == pRequestCmd->RevRequestFlag)
      {
        switch((pRequestCmd->CmdType) & 0xF0)
        {
        case 0xB0: pRequestCmd->AckCmdCode = 0xAB;
                   break;
        case 0xC0: pRequestCmd->AckCmdCode = 0xAC;
                   break;
        case 0xD0: pRequestCmd->AckCmdCode = 0xAD;
        
                   if(0xD2 == pRequestCmd->CmdType)
                   {
                      gSendDoorBoardALogFlag = 1;
                   }
                   break;
        case 0xE0: pRequestCmd->AckCmdCode = 0xAE;
                   break;
        case 0xF0: pRequestCmd->AckCmdCode = 0xAF;
                   break;
        
        
        default: state = DS_NOCMD; break;
        
        }
        
        if(1 == gSendDoorBoardALogFlag)
        {
          for(i = 0; i < 31; i++)
          {
            DoorBoardALogBuffer[i] = DoorBoardCmdBuffer[i];
          }
          gSendDoorBoardALogFlag = 2;
        }
        
        DS_AckRequestCmdFromDoorBoard(pRequestCmd);
        
        pRequestCmd->HandingFlag      = 0;
        pRequestCmd->RevRequestFlag   = 0;
        pRequestCmd->DataLength       = 0;
        pRequestCmd->DataLengthHight  = 0;
        pRequestCmd->DataLengthLow    = 0;
        pRequestCmd->RevDataCnt       = 0;
        pRequestCmd->TotalLength      = 0;
      }
      
      return state;   
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_TrySend5TimesCmdToCoreBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef 
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef  DS_TrySend5TimesCmdToCoreBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      if(1 == pRequestCmd->RevOrSendFlag)
      {
        if(1 == pRequestCmd->RevEchoFlag)
        {
          pRequestCmd->HandingFlag    = 0;
          pRequestCmd->RevOrSendFlag  = 0;
          pRequestCmd->SendTimesCnt   = 0;
          pRequestCmd->RevEchoFlag    = 0;
          pRequestCmd->RevRequestFlag   = 0;
          pRequestCmd->DataLength       = 0;
          pRequestCmd->DataLengthHight  = 0;
          pRequestCmd->DataLengthLow    = 0;
          pRequestCmd->RevDataCnt       = 0;
          pRequestCmd->TotalLength      = 0;
        }
        else
        {
          if(pRequestCmd->SendTimesCnt > 4)
          {
            //超时错误
            pRequestCmd->HandingFlag = 0;
            pRequestCmd->RevOrSendFlag = 0;
            pRequestCmd->SendTimesCnt = 0;
            pRequestCmd->RevEchoFlag = 0;
            pRequestCmd->RevRequestFlag   = 0;
            pRequestCmd->DataLength       = 0;
            pRequestCmd->DataLengthHight  = 0;
            pRequestCmd->DataLengthLow    = 0;
            pRequestCmd->RevDataCnt       = 0;
            pRequestCmd->TotalLength      = 0;
          }
          else
          {
            state = DS_SendDataToCoreBoard(CoreBoardCmdBuffer,pRequestCmd->TotalLength,0xFFFF);
            (pRequestCmd->SendTimesCnt)++;
          }
        }
      }
      return state;  
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_TrySend5TimesCmdToDoorBoard()
    *
    *       Input           :pPROTOCOLCMD pRequestCmd
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_TrySend5TimesCmdToDoorBoard(pPROTOCOLCMD pRequestCmd)
    {
      DS_StatusTypeDef state = DS_OK;
      if(1 == pRequestCmd->RevOrSendFlag)
      {
        if(1 == pRequestCmd->RevEchoFlag)
        {
          pRequestCmd->HandingFlag    = 0;
          pRequestCmd->RevOrSendFlag  = 0;
          pRequestCmd->SendTimesCnt   = 0;
          pRequestCmd->RevEchoFlag    = 0;
          pRequestCmd->RevRequestFlag   = 0;
          pRequestCmd->DataLength       = 0;
          pRequestCmd->DataLengthHight  = 0;
          pRequestCmd->DataLengthLow    = 0;
          pRequestCmd->RevDataCnt       = 0;
          pRequestCmd->TotalLength      = 0;
        }
        else
        {
          if(pRequestCmd->SendTimesCnt > 4)
          {
            //超时错误
            pRequestCmd->HandingFlag = 0;
            pRequestCmd->RevOrSendFlag = 0;
            pRequestCmd->SendTimesCnt = 0;
            pRequestCmd->RevEchoFlag = 0;
            pRequestCmd->RevRequestFlag   = 0;
            pRequestCmd->DataLength       = 0;
            pRequestCmd->DataLengthHight  = 0;
            pRequestCmd->DataLengthLow    = 0;
            pRequestCmd->RevDataCnt       = 0;
            pRequestCmd->TotalLength      = 0;
          }
          else
          {
            state = DS_SendDataToDoorBoard(DoorBoardCmdBuffer,pRequestCmd->TotalLength,0xFFFF);
            (pRequestCmd->SendTimesCnt)++;
          }
        }
      }   
      return state; 
    }
    
    /*******************************************************************************
    *
    *       Function        :DS_Test()
    *
    *       Input           :void
    *
    *       Return          :DS_StatusTypeDef
    *
    *       Description     :--
    *
    *
    *       Data            :2017/12/26
    *       Author          :bertz
    *******************************************************************************/
    DS_StatusTypeDef DS_Test(void)
    {
      DS_StatusTypeDef state = DS_OK;
      
      
      return state;
    }
    
    DS_StatusTypeDef DS_SendHelpCmd(void)
    {
      DS_StatusTypeDef state = DS_OK;
      uint8_t i;
      uint8_t temp[7];
      temp[0] = 0x5B;
      temp[1] = 0xE3;
      temp[2] = 0x01;
      temp[3] = 0x00;
      temp[4] = 0x00;
      temp[5] = 0x00;
      temp[6] = 0x5D;
      
      temp[5] = temp[1];
      for(i = 0; i < 3; i++)
      {
        temp[5] ^=temp[2 + i];
      }
      state = (DS_StatusTypeDef)HAL_UART_Transmit(&huart1, temp, 7,0xFFFF);
      return state;
    }
  /**
  * @}
  */
  /**
  * @}
  */
  /*****************************END OF FILE**************************************/
