/**
  ******************************************************************************
  * File Name          : ds_protocol.h
  * Description        : this file is to generiation protocol codes
  *
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ds_protocol_H
#define __ds_protocol_H
#ifdef __cplusplus
 extern "C" {
#endif
   /* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "usart.h"
   
#define DS_CMD_LEN                    5
#define DS_RX_LEN                     2048      //Accept the buffer size
#define DS_DATA_LEN                   2048      //
#define REQUESTFIXEDCOMMANDLEN        7         //Header + CmdType + CmdParam + DataLength + XOR8Bits + End
#define ACKFIXEDCOMMANDLEN            6         //Header + AckCmdCode + AckCodeH + XOR8Bits  + End
   
   /** enum: DS_StatusTypeDef
   **
   ** DESCRIPTION:
   **  --«˝∂Ø∞Â¿‡–Õ
   **
   ** CREATED: 2017/12/7, by bert
   **
   ** FILE: DS_Protocol.h
   **
   ** AUTHOR: Bert.Zhang
   ********************************************************************************
   */
   typedef enum
   {
     DS_OK       = 0x00U,
     DS_ERROR    = 0x01U,
     DS_BUSY     = 0x02U,
     DS_TIMEOUT  = 0x03U,
     DS_NOCMD    = 0x04U
   }DS_StatusTypeDef;
   
   /*******************************************************************************
   ** struct: sUsartReciveType
   **
   ** DESCRIPTION:
   **  --
   **
   ** CREATED: 2017/12/7, by Bert
   **
   ** FILE: DS_Protocol.h
   **
   ** AUTHOR: Bert.Zhang
   ********************************************************************************
   */
   struct sUsartReciveType
   {
     uint8_t     RX_Flag:1;
     uint16_t    RX_Size;
     uint8_t     RX_pData[DS_RX_LEN];
   };  
   /*******************************************************************************
   ** struct: sProtocolCmd
   **
   ** DESCRIPTION:
   **  -- new ProtocolCmd 
   **
   ** CREATED: 2017/12/7, by Bert
   **
   ** FILE: DS_Protocol.h
   **
   ** AUTHOR: Bert.Zhang
   *****************************************************************************
   */
   struct sProtocolCmd
   {
     uint8_t     CmdType;
     uint8_t     CmdParam;
     uint8_t     DataLengthLow;
     uint8_t     DataLengthHight;
     uint8_t     XOR8BIT;
     uint16_t    DataLength;
     uint16_t    TotalLength;
     
     uint8_t     HandingFlag;
     uint8_t     AckCmdCode;
     uint8_t     AckCodeH;
     uint8_t     AckCodeL;
     uint8_t     AckXOR8BIT;
     
     uint16_t    RevDataCnt;
     uint8_t     RevOrSendFlag;
     uint8_t     RevRequestFlag;
     uint8_t     RevEchoFlag;
     uint8_t     SendTimesCnt;
   };
   
   typedef struct sProtocolCmd       PROTOCOLCMD,        *pPROTOCOLCMD;
   typedef struct sUsartReciveType   USARTRECIVETYPE,    *pUSARTRECIVETYPE;
   
   DS_StatusTypeDef DS_SendOPenDoorCmd(void);
   DS_StatusTypeDef DS_CoreBoardProtocolInit(void);
   DS_StatusTypeDef DS_DoorBoardProtocolInit(void);
   DS_StatusTypeDef DS_SendDataToCoreBoard(uint8_t* pData, uint16_t size, uint32_t Timeout);
   DS_StatusTypeDef DS_SendDataToDoorBoard(uint8_t* pData, uint16_t size, uint32_t Timeout);
   
   DS_StatusTypeDef DS_HandingUartDataFromCoreBoard(void);
   DS_StatusTypeDef DS_HandingUartDataFromDoorBoard(void);   
   
   DS_StatusTypeDef DS_SendRequestCmdToCoreBoard(pPROTOCOLCMD pRequestCmd);
   DS_StatusTypeDef DS_SendRequestCmdToDoorBoard(pPROTOCOLCMD pRequestCmd);
   
   DS_StatusTypeDef DS_AckRequestCmdFromCoreBoard(pPROTOCOLCMD pRequestCmd);
   DS_StatusTypeDef DS_AckRequestCmdFromDoorBoard(pPROTOCOLCMD pRequestCmd);
   
  DS_StatusTypeDef DS_HandingCmdFromCoreBoard(pPROTOCOLCMD pRequestCmd);
  DS_StatusTypeDef DS_HandingCmdFromDoorBoard(pPROTOCOLCMD pRequestCmd);
  
  DS_StatusTypeDef DS_TrySend5TimesCmdToCoreBoard(pPROTOCOLCMD pRequestCmd);
  DS_StatusTypeDef DS_TrySend5TimesCmdToDoorBoard(pPROTOCOLCMD pRequestCmd);
  
  
  DS_StatusTypeDef DS_Test(void);
   
   
   
   
  
#ifdef __cplusplus
}
#endif
#endif /*__ds_protocol_H */