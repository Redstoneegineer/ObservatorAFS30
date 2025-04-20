/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    UART/UART_HyperTerminal_IT/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use UART HAL and LL APIs to transmit
  *          and receive a data buffer with a communication process based on IT;
  *          The communication is done with the Hyperterminal PC application;
  *          HAL driver is used to perform UART configuration, 
  *          then TX/RX transfers procedures are based on LL APIs use
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* UART handler declaration */
UART_HandleTypeDef UartHandle;
__IO uint8_t  ubTxComplete = 0;
__IO uint8_t  ubRxComplete = 0;

/* Buffer used for transmission */
uint8_t aTxStartMessage[] = "\n\r ****UART-Hyperterminal communication based on IT (Mixed HAL/LL usage) ****\n\r Enter 10 characters using keyboard :\n\r";
__IO uint32_t uwTxIndex = 0;
uint8_t ubSizeToSend = sizeof(aTxStartMessage);
uint8_t aTxEndMessage[] = "\n\r Example Finished\n\r";

/* Buffer used for reception */
char aRxBuffer[MAX_NMEA_LENGTH];
__IO uint32_t uwRxIndex = 0;

// Error counters
uint32_t checksum_errors = 0;
uint32_t invalid_char_errors = 0;
uint32_t field_length_errors = 0;
uint32_t timeout_errors = 0;
uint32_t tag_format_errors = 0;
uint32_t tag_checksum_errors = 0;
uint32_t tag_syntax_errors = 0;
uint32_t tag_framing_errors = 0;

// Keep track of last receive time
uint32_t last_receive_time_ms = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
/* STM32H5xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Configure leds */
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED3);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */

  /*## Configure UART peripheral for reception process (using LL) ##########*/  
  /* Any data received will be stored "aRxBuffer" buffer : the number max of 
     data received is RXBUFFERSIZE */
  /* Enable RXNE and Error interrupts */  
  LL_USART_EnableIT_RXNE(USART3);
  LL_USART_EnableIT_ERROR(USART3);

  /*## Start the transmission process (using LL) *##########################*/  
  /* While the UART in reception process, user can transmit data from 
     "aTxStartMessage" buffer */
  /* Start USART transmission : Will initiate TXE interrupt after TDR register is empty */
  LL_USART_TransmitData8(USART3, aTxStartMessage[uwTxIndex++]); 

  /* Enable TXE interrupt */
  LL_USART_EnableIT_TXE(USART3); 

  /*## Wait for the end of the transfer ###################################*/
  /* USART IRQ handler is not anymore routed to HAL_UART_IRQHandler() function 
     and is now based on LL API functions use. 
     Therefore, use of HAL IT based services is no more possible. */
  /*  Once TX transfer is completed, LED2 will turn On.
      Then, when RX transfer is completed, LED1 will turn On. */
  while (ubTxComplete == 0)
  {
  }
  BSP_LED_On(LED2);
   
  while (ubRxComplete == 0)
  {
  }
  BSP_LED_On(LED1);


  /*## Send the received Buffer ###########################################*/
  /* Even if use of HAL IT based services is no more possible, use of HAL Polling based services
     (as Transmit in polling mode) is still possible. */
  if(HAL_UART_Transmit(&huart3, (uint8_t*)aRxBuffer, MAX_NMEA_LENGTH, 1000)!= HAL_OK)
  {
    /* Transfer error in transmission process */
    Error_Handler();
  }
  
  /*## Send the End Message ###############################################*/  
  if(HAL_UART_Transmit(&huart3, (uint8_t*)aTxEndMessage, TXENDMESSAGESIZE, 1000)!= HAL_OK)
  {
    /* Transfer error in transmission process */
    Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIGITAL;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 250;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// Utility to get system time in ms
uint32_t get_current_time_ms(void)
{
    return HAL_GetTick(); // from STM32 HAL, returns ms since boot
}

// Compute XOR checksum between '$' and '*'
uint8_t calculate_nmea_checksum(const char *msg)
{
    uint8_t checksum = 0;
    if (*msg == '$') msg++; // skip the starting $

    while (*msg && *msg != '*') {
        checksum ^= *msg++;
    }
    return checksum;
}

// Validate TAG block if present (e.g., starts with `\` and ends with `\`)
bool validate_tag_block(const char *msg)
{
    if (msg[0] != '\\') {
        tag_framing_errors++;
        return false;
    }

    const char *end = strchr(msg + 1, '\\');
    if (!end) {
        tag_framing_errors++;
        return false;
    }

    size_t len = end - msg + 1;
    if (len >= MAX_TAG_LENGTH) {
        tag_syntax_errors++;
        return false;
    }

    // Basic format and checksum check can go here (TAG*hh\)
    const char *star = strchr(msg, '*');
    if (!star || star > end - 3) {
        tag_format_errors++;
        return false;
    }

    uint8_t calc = 0;
    for (const char *p = msg + 1; p < star; p++) {
        calc ^= *p;
    }

    char hex[3] = { star[1], star[2], 0 };
    uint8_t provided = (uint8_t)strtol(hex, NULL, 16);

    if (calc != provided) {
        tag_checksum_errors++;
        return false;
    }

    return true;
}

bool Validate_NMEA_Message(const char *NMEA_MESSAGE)
{
    // Check for transmission timeout
    uint32_t now = get_current_time_ms();
    if (now - last_receive_time_ms > NMEA_TIMEOUT_MS) {
        timeout_errors++;
        last_receive_time_ms = now;
        return false;
    }
    last_receive_time_ms = now;

    const char *sentence = NMEA_MESSAGE;

    // Check and validate optional TAG block
    if (*NMEA_MESSAGE == '\\') {
        if (!validate_tag_block(NMEA_MESSAGE)) {
            return false;
        }

        // Find end of TAG block
        const char *end_tag = strchr(NMEA_MESSAGE + 1, '\\');
        if (!end_tag) return false;
        sentence = end_tag + 1;
    }

    // Check if sentence starts with $ and has checksum *
    if (*sentence != '$') return false;

    const char *star = strchr(sentence, '*');
    if (!star || strlen(star) < 3) {
        checksum_errors++;
        return false;
    }

    // Calculate and compare checksum
    uint8_t calc = calculate_nmea_checksum(sentence);
    char hex[3] = { star[1], star[2], 0 };
    uint8_t expected = (uint8_t)strtol(hex, NULL, 16);

    if (calc != expected) {
        checksum_errors++;
        return false;
    }

    // Check for invalid characters
    for (const char *p = sentence; *p && *p != '*'; p++) {
        if (!isprint((unsigned char)*p) && *p != '\r' && *p != '\n') {
            invalid_char_errors++;
            return false;
        }
    }

    // Check field lengths
    char copy[MAX_NMEA_LENGTH];
    strncpy(copy, sentence, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *token = strtok(copy + 1, ","); // Skip '$'
    int field_count = 0;

    while (token) {
        if (strlen(token) > 20) { // arbitrary field limit
            field_length_errors++;
            return false;
        }
        token = strtok(NULL, ",");
        field_count++;
    }

    if (field_count < 1 || field_count > 30) {
        field_length_errors++;
        return false;
    }

    return true;
}


/**
  * @brief  Rx Transfer completed callback
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void UART_CharReception_Callback(void)
{
	/* Read received character. RXNE flag is cleared by reading RDR */
    char receivedChar = (char)LL_USART_ReceiveData8(USART3);


	if (uwRxIndex < MAX_NMEA_LENGTH - 1) // Leave space for null terminator
	{
		aRxBuffer[uwRxIndex++] = receivedChar;

		if (receivedChar == '\n') // End of message detected
		{
		  aRxBuffer[uwRxIndex] = '\0'; // Null-terminate the string
		  ubRxComplete = 1;            // Set flag to indicate message is ready
		  uwRxIndex = 0;               // Reset for next message
		}
	}
	else
	{
		// Optional: handle buffer overflow
		aRxBuffer[MAX_NMEA_LENGTH - 1] = '\0';
		ubRxComplete = 1;
		uwRxIndex = 0;
	}
	if (ubRxComplete)
	{
	    if (Validate_NMEA_Message(aRxBuffer))
	    {
	        printf("Valid NMEA message: %s", aRxBuffer);
	    }
	    else
	    {
	        printf("Invalid NMEA message\n");
	    }
	    ubRxComplete = 0;
	}
}


/**
 * @brief	Function to interpret and handle received NMEA messages
 */


/**
  * @brief  Function called for achieving next TX Byte sending
  * @retval None
  */
void UART_TXEmpty_Callback(void)
{
  if(uwTxIndex == (ubSizeToSend - 1))
  {
    /* Disable TXE interrupt */
    LL_USART_DisableIT_TXE(USART3);
    
    /* Enable TC interrupt */
    LL_USART_EnableIT_TC(USART3);
  }

  /* Fill TDR with a new char */
  LL_USART_TransmitData8(USART3, aTxStartMessage[uwTxIndex++]);
}

/**
  * @brief  Function called at completion of last byte transmission
  * @retval None
  */
void UART_CharTransmitComplete_Callback(void)
{
  if(uwTxIndex == sizeof(aTxStartMessage))
  {
    uwTxIndex = 0;
    
    /* Disable TC interrupt */
    LL_USART_DisableIT_TC(USART3);
    
    /* Set Tx complete boolean to 1 */
    ubTxComplete = 1;
  }
}

/**
  * @brief  UART error callbacks
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void UART_Error_Callback(void)
{
  __IO uint32_t isr_reg;

  /* Disable USARTx_IRQn */
  NVIC_DisableIRQ(USART3_IRQn);
  
  /* Error handling example :
    - Read USART ISR register to identify flag that leads to IT raising
    - Perform corresponding error handling treatment according to flag
  */
  isr_reg = LL_USART_ReadReg(USART3, ISR);
  if (isr_reg & LL_USART_ISR_NE)
  {
    /* Turn LED3 on: Transfer error in reception/transmission process */
    BSP_LED_On(LED3);
  }
  else
  {
    /* Turn LED3 on: Transfer error in reception/transmission process */
    BSP_LED_On(LED3);
  }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* Toggle LED3 for error */
  while(1)
  {
    BSP_LED_Toggle(LED3);
    HAL_Delay(1000);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
