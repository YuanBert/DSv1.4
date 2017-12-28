/**
  ******************************************************************************
  * File Name          : ds_gentlesensor.c
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
#include "ds_gentlesensor.h"
extern PROTOCOLCMD  gCoreBoardProtocolCmd;
extern PROTOCOLCMD  gDoorBoardProtocolCmd;
extern GPIOSTATUSDETECTION gGentleSensorStatusDetection;

/*******************************************************************************
*
*       Function        :DS_GentleSensorInit()
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
DS_StatusTypeDef DS_GentleSensorInit(void)
{
  DS_StatusTypeDef state = DS_OK;
  gGentleSensorStatusDetection.GpioFilterCntSum = 20;
  gGentleSensorStatusDetection.GpioValidLogicTimeCnt = 0;
  return state; 
}

/*******************************************************************************
*
*       Function        :DS_GentleSensorCheck()
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
DS_StatusTypeDef DS_GentleSensorCheck(void)
{
  DS_StatusTypeDef state = DS_OK;
  if(0 == gGentleSensorStatusDetection.GpioCheckedFlag || gGentleSensorStatusDetection.GpioValidLogicTimeCnt < 100)
  {
    /* Turn off the flash if the car leaves or if the flash blinks longer than the set value */
    DS_LED_OUT_OFF();
    
  }
  
  if(0 == gGentleSensorStatusDetection.GpioSendDataFlag && gGentleSensorStatusDetection.GpioCheckedFlag)
  {
    /* if the vehcile is still ,carry out the release operation */
    
    
    if(gGentleSensorStatusDetection.GpioValidLogicTimeCnt < 100)
    {
      /* Report vehicle timeout notification */
    }
    
  }
  
  if(gGentleSensorStatusDetection.GpioStatusVal && gGentleSensorStatusDetection.GpioSendDataFlag)
  {
    gGentleSensorStatusDetection.GpioValidLogicTimeCnt = 90100;
    /* Flash open*/
    DS_LED_OUT_ON();
    
    /* Report the arrival of car */
    gCoreBoardProtocolCmd.CmdType    = 0xB1;
    gCoreBoardProtocolCmd.CmdParam   = 0x01;
    gCoreBoardProtocolCmd.DataLength = 0x0000;
    gCoreBoardProtocolCmd.DataLengthLow    = 0x00;
    gCoreBoardProtocolCmd.DataLengthHight  = 0x00;
    state = DS_SendRequestCmdToCoreBoard(&gCoreBoardProtocolCmd);
    
    gGentleSensorStatusDetection.GpioSendDataFlag = 0;
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
