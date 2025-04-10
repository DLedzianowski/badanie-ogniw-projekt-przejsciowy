/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*
 * 	PINOUT SPI1
 * 	MISO	PA6
 * 	MOSI 	PB5
 * 	CLK  	PA5
 *
 * 	OLED +3.3V
 *	--CS	PF13
 *	--RST	PF14
 *	--DC	PF15
 *	BME	+3.3V
 *	--CS	PE9
 *	SD card	+3.3V
 *	--CS	PE11
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"
#include "BMPXX80.h"
#include "fatfs_sd.h"
//#include "fatfs_sd_defs.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define RETRY_DELAY_MS 500
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float BMP280temperature = 0;
int32_t BMP280pressure = 0;

FATFS fs;
FIL fil;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void SDcardInit(char* folder_name);
void SDcardWriteData(float *temperature, int32_t *pressure);
void SDcardClose(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void SDcardInit(char* folder_name) {
    FRESULT res;
    uint8_t retry_count = 5;

    while (retry_count--) {
        res = f_mount(&fs, "", 1);
        if (res == FR_OK) {
            break;
        }
        printf("Error mounting filesystem! (%d). Retrying...\n", res);
        HAL_Delay(RETRY_DELAY_MS);
    }

    retry_count = 5;
    while (retry_count--) {
        res = f_open(&fil, "file.txt", FA_OPEN_ALWAYS | FA_WRITE);
        if (res == FR_OK) {
            break;
        }
        printf("Error opening SDcard file! (%d). Retrying...\n", res);
        HAL_Delay(RETRY_DELAY_MS);
    }

    res = f_lseek(&fil, f_size(&fil));
    if (res != FR_OK) {
        printf("Error seeking to end of file! (%d)\n", res);
        f_close(&fil);
        return;
    }

    f_puts("\n--- Nowy pomiar ---\n", &fil);
    f_puts("Temperatura,Cisnienie\n", &fil);

    f_sync(&fil);

}

void SDcardWriteData(float *temperature, int32_t *pressure) {
    if (f_lseek(&fil, f_size(&fil)) != FR_OK) {
        printf("Error seeking in file!\n");
        ST7735_WriteString(10, ST7735_WIDTH-20, "Error in file!", Font_7x10, ST7735_RED, ST7735_BLACK);
        return;
    }

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%.2f,%ld\n", *temperature, *pressure);

    if (f_puts(buffer, &fil) < 0) {
        printf("Error writing to file!\n");
    }

    if (f_sync(&fil) != FR_OK) {
        printf("Error syncing file!\n");
    }
    f_sync(&fil);
}


void SDcardClose(void) {
    if (f_close(&fil) != FR_OK) {
        printf("Error closing file!\n");
    }
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
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  ST7735_Init();
  ST7735_FillScreen(ST7735_BLACK);

  BMP280_Init(&hspi1, BMP280_TEMPERATURE_16BIT, BMP280_STANDARD, BMP280_FORCEDMODE);

  SDcardInit("test.txt");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  	BMP280_ReadTemperatureAndPressure(&BMP280temperature, &BMP280pressure);
  	printf("Temperature: %.2f °C, %ld Pa\n\r", BMP280temperature, BMP280pressure);



  	SDcardWriteData(&BMP280temperature, &BMP280pressure);



    char buffer[100];
    int tempInt = (int)(BMP280temperature * 100);
    int tempFrac = tempInt % 100;

    // Temperatura
    int len = snprintf(NULL, 0, "Temp: %d.%02d °C", tempInt / 100, tempFrac) + 1;
    if (len < sizeof(buffer)) {
    	snprintf(buffer, sizeof(buffer), "Temp: %d.%02d °C", tempInt / 100, tempFrac);
    	ST7735_WriteString(10, 10, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);
    }
    ST7735_WriteString(10, 10, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);

    // Cisnienie
    len = snprintf(NULL, 0, "Pressure: %ld Pa", BMP280pressure) + 1;
    if (len < sizeof(buffer)) {
    	snprintf(buffer, sizeof(buffer), "Prs: %ld Pa", BMP280pressure);
    	ST7735_WriteString(10, 20, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);
    }
    ST7735_WriteString(10, 20, buffer, Font_7x10, ST7735_WHITE, ST7735_BLACK);


  	HAL_Delay(1000);
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
