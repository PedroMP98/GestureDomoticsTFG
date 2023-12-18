/* USER CODE BEGIN Header */
/**
 ******************************************************************************
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  * Autor: PEDRO MELÉNDEZ DEL POZO - 53404                                 //
//                                                                           //
//  * Archivo: main.cpp                                                      //
//                                                                           //
//  * Tema:  TFG - Control gestual de una cortina domotizada                 //
//                 para aplicaciones domoticas                               //
//                                                                           //
//  * Versión: 1.0                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
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

#include "APDS9960.h" //APDS-9960 Gesture Sensor
#include "string.h"   //Convert address to string
#include "stdio.h"    //Sprintf
#include "stdbool.h"  //Bool

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

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
APDS9960 apds = APDS9960();//Object APDS-9960

volatile bool isr_gesture_flag = false; //It is activated when it receives an interruption

//The following variables help us to determine the movement that produces the interruption
volatile bool UP_flag = false;
volatile bool DOWN_flag = false;
volatile bool LEFT_flag = false;
volatile bool RIGHT_flag = false;
volatile bool FAR_flag = false;
volatile bool NEAR_flag = false;
volatile bool NONE_flag = false;



//Engine status 1-ON/0-OFF
int status_UP = 0;
int status_DOWN = 0;
unsigned long time;
unsigned long Tmax = 7000;//Waiting time to disable motors

//Functions
void handleGesture();//Gesture handler
void motor_up();//Raise blind
void motor_down();//Lower blind
void stop_system();//Stom motors

void send_uart_to_transmit(char* text) //Send data using UART
{
    HAL_UART_Transmit(&huart2, (uint8_t*)text, strlen(text), HAL_MAX_DELAY);
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//ISR
{

	if (GPIO_Pin == GPIO_PIN_0)
		isr_gesture_flag = true;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//I2C scanner
	uint8_t ack; // Recognition bit
	char dir[100];
	int nd = 0;    //List of I2C devices found
	int counter = HAL_GetTick(); //Timer
	int button_count=0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //////////////  I2C DEVICE SCANNER ////////////////////////////////////////////////////
  send_uart_to_transmit(" Scanning I2C devices\n");
  for(uint8_t adress=0;adress<128;adress++) //4 bits => 128 possible addresses
  {
	  ack = HAL_I2C_IsDeviceReady(&hi2c1,adress<<1,3,HAL_MAX_DELAY);
	  //The slave responds if it is its address.
	  if(ack == HAL_OK)
	  {
		  nd++;
		  sprintf((char*)dir," Device %d: 0x%X  \n",nd,adress); //Convert to string
		  HAL_UART_Transmit(&huart2,(uint8_t*)dir,strlen(dir),HAL_MAX_DELAY);
	  }
  }
  /////////////////////////////////////////////////
  if(apds.init()==true) //INITIALIZATION: APDS-9960
  {
	  send_uart_to_transmit("\n APDS9960 initialized successfully\n");
	  HAL_Delay(100);

  }
  else
  {
	  send_uart_to_transmit("\n ..ERROR.. Failed to initialize APDS9960\n");
	  HAL_Delay(100);

  }

  if(apds.enableGestureSensor(true)) //INITIALIZATION: GESTURE MODE
  {
	  send_uart_to_transmit(" Gesture detection mode activated \n\n");
	  HAL_Delay(100);

  }
  else
  {
	  send_uart_to_transmit(" ..ERROR.. Could not start gesture detection mode.\n\n");
	  HAL_Delay(100);

  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if ( isr_gesture_flag == true )
	  {
		  HAL_NVIC_DisableIRQ(EXTI0_IRQn); //Disable interrupts
		  handleGesture();
		  isr_gesture_flag = false;

	  }

	  if ((status_UP==1 || status_DOWN==1) && (HAL_GetTick()-time)>Tmax) //Disable motors after a set time
	       stop_system();

	  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3) == 1)
	  {
		  stop_system();
		  HAL_Delay(150);
		  motor_up();
		  send_uart_to_transmit("\n UP BUTTON ON \n");
		  while (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3) == 1)
		  {
			  //Raise blinds
		  }
		  send_uart_to_transmit("\n UP BUTTON OFF \n");
		  stop_system();
	  }

	  else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4) == 1)
	  {
		  stop_system();
		  HAL_Delay(150);
		  motor_down();
		  send_uart_to_transmit("\n DOWN BUTTON ON \n");
		  while (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4) == 1)
		  {
			  //Lower blinds
		  }
		  send_uart_to_transmit("\n DOWN BUTTON OFF \n");
		  stop_system();
	  }


	  else if (UP_flag == true) //Gesture raise blinds
	  {
		  send_uart_to_transmit("\n RISING BLIND \n");
		  motor_up();
		  UP_flag = false;
	  }
	  else if (DOWN_flag == true)//Gesture lower blinds
		  {
		  send_uart_to_transmit("\n LOWERING BLIND\n");
		  motor_down();
		  DOWN_flag = false;

	  }
	  else if (LEFT_flag == true) //Leftward movement
	  {
		  send_uart_to_transmit("\n LEFTWARD MOVEMENT <-- \n");
		  LEFT_flag = false;
	  }
	  else if (RIGHT_flag == true) //Rightward movement
	  {
		  send_uart_to_transmit("\n RIGHTWARD MOVEMENT --> \n");
		  RIGHT_flag = false;
	  }
	  else if (NEAR_flag == true) //Movimiento acercar
	  {
		  send_uart_to_transmit("\n SYSTEM STOP \n");
		  stop_system();
		  NEAR_flag = false;
	  }
	  else if(NONE_flag == true )
	  {
		  send_uart_to_transmit("\n NONE GESTURE. \n");
		  NONE_flag = false;
	  }
	  else if(FAR_flag == true)
	  {
		  send_uart_to_transmit("\n FAR GESTURE. \n");
		  FAR_flag = false;
	  }

	  HAL_NVIC_EnableIRQ(EXTI0_IRQn);  //Enable interrupts
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void handleGesture() //Gesture handler
{
  if ( apds.isGestureAvailable() )
  {
    switch ( apds.readGesture() )
    {
      case DIR_UP:
        UP_flag = true;
        break;

      case DIR_DOWN:
        DOWN_flag = true;
        break;

      case DIR_LEFT:
        LEFT_flag = true;
        break;

      case DIR_RIGHT:
        RIGHT_flag = true;
        break;

      case DIR_NEAR:
        NEAR_flag = true;
        break;

      case DIR_FAR:
        FAR_flag = true;
        break;

      default:
        NONE_flag = true;
        break;
    }
  }
}

void motor_up() //Raise blind function
{
	stop_system();
	HAL_Delay(150);
	status_UP = 1;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
	time = HAL_GetTick();
}

void motor_down() //Lower blind function
{
	stop_system();
	HAL_Delay(150);
	status_DOWN = 1;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	time = HAL_GetTick();
}

void stop_system() //Stom motors function
{
	status_UP = 0;
	status_DOWN = 0;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
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
		__disable_irq();
		while (1)
		{
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
