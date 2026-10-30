/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED_I2C.h"
#include <stdio.h>
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

/* USER CODE BEGIN PV */

//定义结构体
typedef struct{
	
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	
}Clock_t;

//当前时间

Clock_t currentTime = {2026,10,29,6,1,15};


//毫秒计数器

volatile uint16_t ms_tick = 0;




typedef enum {
    MODE_NORMAL = 0, // 正常走时
    MODE_SET_HOUR,   // 调小时
    MODE_SET_MIN,    // 调分钟
    MODE_SET_ALARM ,  // 调闹钟
	
	 MODE_SET_ALARM_H, // 调闹钟小时
    MODE_SET_ALARM_M  // 调闹钟分钟
	
} ClockMode_t;

ClockMode_t current_mode = MODE_NORMAL; 


uint8_t alarm_hour = 12; // 默认闹钟 12点
uint8_t alarm_min = 5;   // 默认闹钟 05分
uint8_t alarm_triggered = 0; // 0:没响，1:正在响


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//查哪个端口（GPIOA)  哪个引脚（GPIO_PIN_0 ）	返回 1 代表“确认按下了”，返回 0 代表“没按或一直按着”
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) 
{
	static uint8_t key_up[16] = {1}; 		//初始化为 {1}：意思是刚上电时，所有按键默认都是松开的状态
	//static（静态变量）
	//加上 static，这个变量就会永远记住上一次的值。下一次再进这个函数，它还是上次的值
//[16]（数组）用 16 个元素的数组，就能分别记住 PA0~PA15 这 16 个引脚各自有没有被按下
	
	
	if (key_up[GPIO_Pin & 0x0F] == 1 && HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 0) {
        HAL_Delay(20); // 消抖
        if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 0) {
            key_up[GPIO_Pin & 0x0F] = 0; // 标记为已按下
            return 1;
        }
    } else if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 1) {
        key_up[GPIO_Pin & 0x0F] = 1; // 松开了，恢复标志
    }
    return 0;
	
	
	
	
}




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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	
OLED_Init();
OLED_CLS();

HAL_TIM_Base_Start_IT(&htim2);	//启动定时器并使能更新中断，计数溢出时进入中断

// 强制拉低 PB12，确保开机不响
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
	char display_buf[30]; // 用于 sprintf

	
	OLED_ShowStr(0, 0, (unsigned char*)"Multi_Clock", 2);
	
  while (1)
  {
		 
		
		// 如果按下了 MODE 键 (PA0)
      if (Key_Scan(GPIOA, GPIO_PIN_0)) {
          current_mode++; // 切换模式
          if (current_mode > MODE_SET_ALARM_M) current_mode = MODE_NORMAL; // 循环
      }
      
      // 如果按下了 ADD 键 (PA1)
      if (Key_Scan(GPIOA, GPIO_PIN_1)) 
      {  
				if (current_mode == MODE_SET_HOUR) 
          {    currentTime.hour++;
						
              if (currentTime.hour >= 24) 
							{
								currentTime.hour = 0;
							}
						
          }
          else if (current_mode == MODE_SET_MIN) 
          {   currentTime.min++;
              if (currentTime.min >= 60)
							{	
								currentTime.min = 0;
							}
          }
					else if(current_mode ==  MODE_SET_ALARM_H)
					{
						alarm_hour++;
							if(alarm_hour>=24)
							{
								alarm_hour = 0;
								
							}
						
					}
					else if(current_mode == MODE_SET_ALARM_M)
					{
						alarm_min++;
							if(alarm_min>=60)
							{
								alarm_min = 0;
							}
						
						
					}
					
					
					
      }
			
		  // 如果按下了 SUB 键 (PA2)
      if (Key_Scan(GPIOA, GPIO_PIN_2)) 
       {  
				if (current_mode == MODE_SET_HOUR) 
          {    currentTime.hour--;
						
              if (currentTime.hour == 0) 
							{
								currentTime.hour = 23;
							}
						
          }
          else if (current_mode == MODE_SET_MIN) 
          {   currentTime.min--;
              if (currentTime.min ==0)
							{	
								currentTime.min = 59;
							}
          }
					else if(current_mode ==  MODE_SET_ALARM_H)
					{
						alarm_hour--;
							if(alarm_hour==0)
							{
								alarm_hour = 23;
								
							}
						
					}
					else if(current_mode == MODE_SET_ALARM_M)
					{
						alarm_min--;
							if(alarm_min==0)
							{
								alarm_min = 59;
							}
						
						
					}
					
					
					
      }
			
		

		
		
		
		
		if(currentTime.sec>=60)
		{
				currentTime.sec = 0;
				currentTime.min++;
					if(currentTime.min>=60)
					{
							currentTime.min = 0;
							currentTime.hour++;
								if(currentTime.hour>=24)
								{
									
									currentTime.hour = 0;
									currentTime.day++;
									
									
								}
						
						
						
						
					}
				
			
			
		}
				
		// 如果当前时间 == 闹钟时间，并且秒数是0（只在每分钟的第一秒响），且还没触发过
      if (current_mode == MODE_NORMAL && 
          currentTime.hour == alarm_hour && 
          currentTime.min == alarm_min && 
          currentTime.sec == 0 && 
          alarm_triggered == 0) 
      {
          alarm_triggered = 1; // 标记正在响铃
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // 蜂鸣器响 (低电平触发)
      }

        // 如果正在响铃，按任意键停止
      if (alarm_triggered == 1) {
          // 直接读引脚电平，只要是低电平（按下了），直接停
          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0 ||
              HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0  ||
              HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0) 
          {
              
              HAL_Delay(20); 
              if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 0 || 
                  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0 || 
                  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0) 
              {
                  alarm_triggered = 0; // 解除标记
                  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // 蜂鸣器关
              }
          }
      }
		
		
		
		
		
		//%04 :  4表示至少占4位，0表示不足4位在前面补0
			sprintf(display_buf, "Alm: %02d:%02d", alarm_hour, alarm_min);
OLED_ShowStr(0, 4, (unsigned char*)display_buf, 2);
			
			
				  sprintf(display_buf, "Date: %04d-%02d-%02d", currentTime.year, currentTime.month, currentTime.day);
      OLED_ShowStr(0, 0, (unsigned char*)display_buf, 2); 
      
     if (current_mode == MODE_SET_ALARM_H || current_mode == MODE_SET_ALARM_M) 
      {
          // 如果在调闹钟模式，就把闹钟变量打印出来，并在前面加个星号提示
          sprintf(display_buf, "Set Alm: %02d:%02d  ", alarm_hour, alarm_min);
      } 
      else 
      {
          // 如果在其他模式，正常显示当前时间
          sprintf(display_buf, "Time: %02d:%02d:%02d", currentTime.hour, currentTime.min, currentTime.sec);
      }
      OLED_ShowStr(0, 2, (unsigned char*)display_buf, 2); 
				
		
		
		// 显示当前模式
      if (current_mode == MODE_NORMAL) {
          OLED_ShowStr(0, 6, (unsigned char*)"Mode: NORMAL   ", 2);
      } else if (current_mode == MODE_SET_HOUR) {
          OLED_ShowStr(0, 6, (unsigned char*)"Mode: SET HOUR ", 2);
      } else if (current_mode == MODE_SET_MIN) {
          OLED_ShowStr(0, 6, (unsigned char*)"Mode: SET MIN  ", 2);
      }
       else if (current_mode == MODE_SET_ALARM_H){
				 OLED_ShowStr(0, 6, (unsigned char*)"Mode: ALM HOUR ", 2);
			 }
      else if (current_mode == MODE_SET_ALARM_M){
				 OLED_ShowStr(0, 6, (unsigned char*)"Mode: ALM MIN  ", 2);
			}
		
		
		
		HAL_Delay(10); // 防止刷屏过快卡死				
		
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

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
if (htim->Instance == TIM2) 
  {
		
		 if (current_mode == MODE_NORMAL)
				{
					ms_tick++;
					if (ms_tick >= 1000) //满了1000ms就是1秒
					{
							ms_tick = 0;
						currentTime.sec++;
					}
					
				}
					
  }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
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
