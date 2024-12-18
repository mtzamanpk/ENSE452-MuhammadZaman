/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // Required for atoi
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Forward declarations
char cliBufferTX[100];
void printStringBlocking(const char *str);
void welcomeMessage(void);
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
UART_HandleTypeDef huart2;

osThreadId FloorSelectionTHandle;
osThreadId ElevatorMovemenHandle;
osThreadId DisplayUpdateTaHandle;
osSemaphoreId emergencySemaphoreHandle;
/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void FloorSelectionTaskHandle(void const * argument);
void ElevatorMovementTaskHandle(void const * argument);
void DisplayUpdateTaskHandle(void const * argument);

/* USER CODE BEGIN PFP */

QueueHandle_t floorQueue;
QueueHandle_t emergencyQueue; // Declare emergencyQueue globally
int currentFloor = 1; // Initial floor
volatile int insideState = 0; // 0 = Outside, 1 = Inside
volatile int emergencyState = 0; // 0 = Normal, 1 = Emergency
SemaphoreHandle_t emergencySemaphore; // Semaphore for emergency mode
SemaphoreHandle_t floorMutex;
SemaphoreHandle_t emergencyMutex;
volatile int emergencyFlag = 0; // Set by ISR, processed by a task
/* USER CODE END PV */


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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE BEGIN 2 */
  floorMutex = xSemaphoreCreateMutex();
  if (floorMutex == NULL) {
      Error_Handler(); // Handle mutex creation failure
  }

  emergencyMutex = xSemaphoreCreateMutex();
  if (emergencyMutex == NULL) {
      Error_Handler(); // Handle mutex creation failure
  }
  /* USER CODE END 2 */

  emergencyQueue = xQueueCreate(1, sizeof(int));
  if (emergencyQueue == NULL) {
      Error_Handler(); // Handle creation failure
  }

  floorQueue = xQueueCreate(2, sizeof(int));
  if (floorQueue == NULL) {
      Error_Handler();
  }


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of emergencySemaphore */
  osSemaphoreDef(emergencySemaphore);
  emergencySemaphoreHandle = osSemaphoreCreate(osSemaphore(emergencySemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of FloorSelectionT */
  osThreadDef(FloorSelectionT, FloorSelectionTaskHandle, osPriorityBelowNormal, 0, 128);
  FloorSelectionTHandle = osThreadCreate(osThread(FloorSelectionT), NULL);

  /* definition and creation of ElevatorMovemen */
  osThreadDef(ElevatorMovemen, ElevatorMovementTaskHandle, osPriorityNormal, 0, 256);
  ElevatorMovemenHandle = osThreadCreate(osThread(ElevatorMovemen), NULL);

  /* definition and creation of DisplayUpdateTa */
  osThreadDef(DisplayUpdateTa, DisplayUpdateTaskHandle, osPriorityAboveNormal, 0, 128);
  DisplayUpdateTaHandle = osThreadCreate(osThread(DisplayUpdateTa), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


void CLI_Transmit(const char *message) {
    HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}



#define RX_BUFFER_SIZE 100
char rx_buffer[RX_BUFFER_SIZE];

int CLI_Receive(char *buffer, uint16_t buffer_size) {
    uint16_t index = 0;
    uint8_t rx_char;

    while (index < buffer_size - 1) {
        if (HAL_UART_Receive(&huart2, &rx_char, 1, HAL_MAX_DELAY) == HAL_OK) {
            HAL_UART_Transmit(&huart2, &rx_char, 1, HAL_MAX_DELAY);
            if (rx_char == '\r' || rx_char == '\n') {
                buffer[index] = '\0';
                return 1; // Successful reception
            }
            buffer[index++] = rx_char;
        } else {
            return 0; // Reception error
        }
    }
    buffer[index] = '\0';
    return 1;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_13 && current_time - last_interrupt_time > 200) { // Debounce: 200 ms
        int emergencySignal = 1;
        xQueueSendFromISR(emergencyQueue, &emergencySignal, NULL);
        last_interrupt_time = current_time;
    }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_FloorSelectionTaskHandle */
/**
  * @brief  Function implementing the FloorSelectionT thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_FloorSelectionTaskHandle */
void FloorSelectionTaskHandle(void const *argument) {
    char command_buffer[100];
    int targetFloor;

    CLI_Transmit("\x1b[2J\x1b[3;1H\x1b[4;1r\x1b[4;1H");

    for (;;) {
        // Process emergency signals
        int emergencySignal;
        if (xQueueReceive(emergencyQueue, &emergencySignal, 0) == pdPASS) {
            xSemaphoreTake(emergencyMutex, portMAX_DELAY);
            emergencyState = 1;
            xSemaphoreGive(emergencyMutex);

            CLI_Transmit("\x1b[2J\x1b[H");
            CLI_Transmit("Emergency mode activated. All operations halted.\n");
        }

        // Display CLI prompt
        CLI_Transmit("\x1b[24;1H> ");
        if (CLI_Receive(command_buffer, sizeof(command_buffer))) {
            CLI_Transmit("\x1b[23;1H");
            CLI_Transmit("Received: ");
            CLI_Transmit(command_buffer);
            CLI_Transmit("\n");

            // Handle emergency mode
            xSemaphoreTake(emergencyMutex, portMAX_DELAY);
            if (emergencyState) {
                xSemaphoreGive(emergencyMutex);
                if (strncmp(command_buffer, "exit_emergency", 14) == 0) {
                    xSemaphoreTake(emergencyMutex, portMAX_DELAY);
                    emergencyState = 0; // Deactivate emergency mode
                    xSemaphoreGive(emergencyMutex);
                    CLI_Transmit("Emergency mode deactivated. Resuming operations.\n");
                } else {
                    CLI_Transmit("Only 'exit_emergency' allowed in emergency mode.\n");
                }
                continue; // Ignore other commands
            }
            xSemaphoreGive(emergencyMutex);

            // Handle normal commands
            if (strncmp(command_buffer, "enter", 5) == 0) {
                insideState = 1;
                CLI_Transmit("You have entered the elevator.\n");
            } else if (strncmp(command_buffer, "exit", 4) == 0) {
                insideState = 0;
                CLI_Transmit("You have exited the elevator.\n");
            } else if (strncmp(command_buffer, "floor=", 6) == 0) {
                if (insideState) {
                    targetFloor = atoi(command_buffer + 6);
                    if (targetFloor >= 1 && targetFloor <= 3) {
                        xQueueSend(floorQueue, &targetFloor, pdMS_TO_TICKS(50));
                        CLI_Transmit("Moving to selected floor...\n");
                    } else {
                        CLI_Transmit("Invalid floor. Enter 1, 2, or 3.\n");
                    }
                } else {
                    CLI_Transmit("You must enter the elevator to select a floor.\n");
                }
            } else {
                // Handle invalid command
                CLI_Transmit("Invalid command. Available commands: enter, exit, floor=<1|2|3>, exit_emergency\n");
            }
        }
    }
}



/* USER CODE BEGIN Header_ElevatorMovementTaskHandle */
/**
* @brief Function implementing the ElevatorMovemen thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ElevatorMovementTaskHandle */
void ElevatorMovementTaskHandle(void const *argument) {
    int targetFloor;

    for (;;) {
        if (xQueueReceive(floorQueue, &targetFloor, portMAX_DELAY) == pdPASS) {
            while (1) {
                xSemaphoreTake(floorMutex, portMAX_DELAY);
                if (currentFloor == targetFloor) {
                    xSemaphoreGive(floorMutex);

                    // Turn on the LED on reaching the target floor
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                    osDelay(500); // Keep the LED on for 500 ms
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    break;
                }
                currentFloor += (currentFloor < targetFloor) ? 1 : -1;
                xSemaphoreGive(floorMutex);

                char move_message[50];
                snprintf(move_message, sizeof(move_message), "Moving... Floor: %d\n", currentFloor);
                CLI_Transmit(move_message);

                osDelay(1000); // Delay for floor movement simulation
            }
            CLI_Transmit("Arrived at Target Floor.\n");
        }
    }
}




/* USER CODE BEGIN Header_DisplayUpdateTaskHandle */
/**
* @brief Function implementing the DisplayUpdateTa thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DisplayUpdateTaskHandle */
void DisplayUpdateTaskHandle(void const *argument) {
    char message[50];

    for (;;) {
        xSemaphoreTake(floorMutex, portMAX_DELAY);
        int localFloor = currentFloor;
        xSemaphoreGive(floorMutex);

        // Save current cursor position
        CLI_Transmit("\x1b[s"); // Save cursor position

        // Update the "Current Floor" display
        snprintf(message, sizeof(message), "\x1b[2;16HCurrent Floor: %d   ", localFloor);
        CLI_Transmit(message);

        // Restore the saved cursor position
        CLI_Transmit("\x1b[u"); // Restore cursor position

        osDelay(1000);
    }
}




/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
