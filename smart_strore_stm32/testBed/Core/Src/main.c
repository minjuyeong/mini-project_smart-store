#include "main.h"

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);


void SetServoAngle(uint8_t angle) {
    // 각도에 따라 듀티 사이클 계산
    uint32_t pulseWidth = (angle * (2000 - 1000) / 360) + 1000; // 1000us ~ 2000us
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulseWidth - 1);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();
    MX_TIM2_Init();

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // PWM 시작

    while (1) {
        for (int angle = 0; angle <= 360; angle += 10) {
            SetServoAngle(angle); // 각도 설정
            HAL_Delay(500); // 0.5초 대기
        }
        for (int angle = 360; angle >= 0; angle -= 10) {
            SetServoAngle(angle); // 각도 설정
            HAL_Delay(500); // 0.5초 대기
        }
    }
}

static void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 84 - 1; // 1MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 20000 - 1; // 20ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    HAL_TIM_PWM_Init(&htim2);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1500 - 1; // 초기 듀티 사이클 (1.5ms)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

static void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */


/* USER CODE BEGIN 4 */

//void stepMotor(int step) {
//    switch (step) {
//        case 0:
//            HAL_GPIO_WritePin(GPIOB, IN1_Pin, GPIO_PIN_SET);
//            HAL_GPIO_WritePin(GPIOB, IN2_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN3_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN4_Pin, GPIO_PIN_RESET);
//            break;
//        case 1:
//            HAL_GPIO_WritePin(GPIOB, IN1_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN2_Pin, GPIO_PIN_SET);
//            HAL_GPIO_WritePin(GPIOB, IN3_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN4_Pin, GPIO_PIN_RESET);
//            break;
//        case 2:
//            HAL_GPIO_WritePin(GPIOB, IN1_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN2_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN3_Pin, GPIO_PIN_SET);
//            HAL_GPIO_WritePin(GPIOB, IN4_Pin, GPIO_PIN_RESET);
//            break;
//        case 3:
//            HAL_GPIO_WritePin(GPIOB, IN1_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN2_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN3_Pin, GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(GPIOB, IN4_Pin, GPIO_PIN_SET);
//            break;
//    }
//}
//
//void rotateMotor(int steps) {
//    for (int i = 0; i < steps; i++) {
//        for (int j = 0; j < 4; j++) {
//            stepMotor(j);
//            HAL_Delay(1); // 스텝 간의 지연 시간
//        }
//    }
//}
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

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
