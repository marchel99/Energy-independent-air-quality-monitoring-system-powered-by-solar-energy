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
 * This software is licensed pod terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "ff.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "epd4in2b.h"
#include "epdpaint.h"
#include "fonts.h"
#include "imagedata.h"
#include "user_interface.h"
#include "globals.h"
#include "ens160.h"
#include "max.h"
#include "bme280.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef int (*CanExitMenuFunction)(void);
CanExitMenuFunction canExitMenu = NULL;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint32_t measurement_number = 1;

// volatile uint8_t buttonState_SW = 0;
RTC_TimeTypeDef time;
RTC_DateTypeDef date;
 FATFS FatFs;  // Fatfs handle
    FIL fil;      // File handle
    FRESULT fres; // Result after operations






float read_adc_value(void);
int get_co_ppm(float voltage);
float get_hcho_ppm(float vs);

volatile uint8_t isSDCardInserted = 0;



// main.c lub inny plik, w którym znajduje się HAL_GPIO_EXTI_Callback
extern int encoderPosition;

volatile uint8_t buzzer_active = 0;
volatile uint32_t last_interrupt_time = 0;
volatile uint8_t GoToMenu = 0;

const uint32_t debounce_time = 200; // czas tłumienia drgań w ms

static struct bme280_t bme280;
static struct bme280_t *p_bme280 = &bme280;

volatile int currentIconIndex = 1; // Globalna zmienna do śledzenia wybranej ikony

DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
extern DFRobot_ENS160_I2C ens160;

int counter = 1;
float batteryLevel = 0;

volatile uint8_t inMenu = 0;
 volatile bool sdNotOk=0;



Epd epd;
Paint paint;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
uint8_t CheckAndRestoreCS2(void);





float process_hcho_measurement(void);
int process_co_measurement(void);




void process_SD_card(const char *data);






void user_delay_ms(uint32_t period);
void print_sensor_data_bme280(struct bme280_t *bme280);
void init_ens160(void);
void read_and_print_ens160_data(void);



float calculate_hcho_ppm(float voltage);

int calculate_co_ppm(float voltage);



int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint8_t len);
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint8_t len);

float read_soc(I2C_HandleTypeDef *hi2c);

// void DisplayIcon(int iconIndex);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t loopCounter = 0; // Licznik impulsów pętli while
uint32_t sdCounter = 0;
#define BME280_OK 0
#define BME280_I2C_ADDR 0x76

// Redirect r to UART
int __io_putchar(int ch)
{
  if (ch == '\n')
  {
    uint8_t ch2 = '\r';
    HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
  }
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return 1;
}


uint8_t CheckAndRestoreCS2(void)
{
    // Zapisz aktualny stan pinu CS_2
    GPIO_PinState original_state = HAL_GPIO_ReadPin(CS_2_GPIO_Port, CS_2_Pin);

    // Ustawienie pinu CS_2 jako wejście z rezystorem podciągającym do masy
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = CS_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(CS_2_GPIO_Port, &GPIO_InitStruct);

    // Sprawdzenie stanu pinu
    uint8_t card_inserted = HAL_GPIO_ReadPin(CS_2_GPIO_Port, CS_2_Pin) == GPIO_PIN_SET ? 1 : 0;

    // Przywrócenie oryginalnej konfiguracji pinu CS_2
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(CS_2_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(CS_2_GPIO_Port, CS_2_Pin, original_state);

    // Zwrócenie wyniku
    return card_inserted;
}




void process_SD_card(const char *data) {
    // Prepare the buffer for data
    char data_buffer[512];

    // Get current date and time
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    // Collect sensor data
    int32_t temperature_raw, pressure_raw, humidity_raw;
    bme280_read_uncomp_pressure_temperature_humidity(&pressure_raw, &temperature_raw, &humidity_raw);
    
    int32_t temperature = bme280_compensate_temperature_int32(temperature_raw); // Temperature in hundredths of degrees Celsius
    uint32_t humidity = bme280_compensate_humidity_int32(humidity_raw); // Humidity in thousandths of %
    uint32_t pressure = bme280_compensate_pressure_int32(pressure_raw); // Pressure in Pa

    uint8_t aqi = DFRobot_ENS160_GetAQI(&ens160);
    uint16_t co2 = DFRobot_ENS160_GetECO2(&ens160);
    uint8_t tvoc = DFRobot_ENS160_GetTVOC(&ens160);
    int co_ppm = process_co_measurement();
    float hcho_ppm = process_hcho_measurement();

    // Format data as a CSV string
    snprintf(data_buffer, sizeof(data_buffer),
        "%02d,%02d,%04d,%02d,%02d,%02d,%.2f,%.2f,%.2f,%d,%d,%d,%d,%.2f\r\n",
        date.Date, date.Month, 2000 + date.Year, time.Hours, time.Minutes, time.Seconds,
        temperature / 100.0, humidity / 1024.0, pressure / 100.0, aqi, co2, tvoc, co_ppm, hcho_ppm);

    // Mount and write to the SD card
    DWORD free_clusters;
    FATFS *fs;

    // Check if the SD card is mounted
    FRESULT res = f_getfree("", &free_clusters, &fs);

    if (res != FR_OK) {
        printf("SD card is not mounted, trying to remount...\r\n");

        // Unmount if it was previously mounted
        f_mount(NULL, "", 0); 

        // Attempt to remount the SD card
        FRESULT fres = f_mount(&FatFs, "", 1); 
        if (fres == FR_OK) {
            printf("SD card remounted successfully!\r\n");
        } else {
            printf("Failed to remount SD card: (%i)\r\n", fres);
            sdNotOk=1;
            return;
        }
    }

    // Open the file to append data
    printf("Opening 'sensor_data.csv' for writing...\r\n");
    FRESULT fres = f_open(&fil, "sensor_data.csv", FA_WRITE | FA_OPEN_APPEND);
    if (fres != FR_OK) {
        printf("Error opening file (FA_WRITE | FA_OPEN_APPEND): (%i)\r\n", fres);
        f_mount(NULL, "", 0);
        return;
    }

    // Check if the file is empty and write the header if it is
    if (f_size(&fil) == 0) {
        const char *header = "DAY,MONTH,YEAR,HOUR,MINUTE,SECOND,TEMPERATURE,HUMIDITY,PRESSURE,AQI,CO2,TVOC,CO,HCHO\n";
        f_write(&fil, header, strlen(header), NULL);
    }

    // Write data to the file
    fres = f_write(&fil, data_buffer, strlen(data_buffer), NULL);
    if (fres != FR_OK) {
        printf("Error writing to file\n");
        f_close(&fil);
        f_mount(NULL, "", 0);
        return;
    }
    printf("Data saved: %s\n", data_buffer);

    // Close the file
    fres = f_close(&fil);
    if (fres != FR_OK) {
        printf("Error closing file after writing: (%i)\r\n", fres);
        f_mount(NULL, "", 0);
        return;
    }

    // Unmount the SD card
    f_mount(NULL, "", 0);
    printf("SD card successfully unmounted!\r\n");
}







void read_adc_values(void)
{
    uint32_t value[2];
    float voltage[2];

    HAL_ADC_Start(&hadc1);

    // Conversion for CO channel
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    value[0] = HAL_ADC_GetValue(&hadc1);

    // Conversion for HCHO channel
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    value[1] = HAL_ADC_GetValue(&hadc1);

    // Convert ADC values to voltage
    voltage[0] = 3.3f * value[0] / 4095.0f;
    voltage[1] = 3.3f * value[1] / 4095.0f;

    // Calculate CO ppm
    int co_ppm = get_co_ppm(voltage[0]);

    // Calculate HCHO ppm
    float hcho_ppm = get_hcho_ppm(voltage[1]);




   








    // Print the results
    printf("CO value=%lu (%.3f V), CO ppm=%d\n", value[0], voltage[0], co_ppm);
    printf("HCHO value=%lu (%.3f V), HCHO ppm=%.1f\n", value[1], voltage[1], hcho_ppm);
}


















int calculate_co_ppm(float voltage)
{
    const float R0 = 90.26f; // Wartość R0 w kΩ
    const float RL = 4.7f;   // Wartość rezystora pull-down w kΩ

    float Rs = RL * (3.3f / voltage - 1.0f);
    float ratio = Rs / R0;

    if (ratio > 0.8f)
        return 5;
    else if (ratio > 0.4f)
        return 10;
    else if (ratio > 0.3f)
        return 20;
    else if (ratio > 0.21f)
        return 50;
    else if (ratio > 0.17f)
        return 100;
    else if (ratio > 0.15f)
        return 150;
    else if (ratio > 0.12f)
        return 500;
    else if (ratio > 0.1f)
        return 1000;
    else
        return 5000;
}

float calculate_hcho_ppm(float voltage)
{
    const float v0 = 0.5f; // Zakładane napięcie bazowe w czystym powietrzu dla HCHO

    float ratio = voltage / v0;

    if (ratio <= 1.2)
        return 0.1;
    else if (ratio > 1.2 && ratio <= 1.4)
        return 0.2;
    else if (ratio > 1.4 && ratio <= 1.6)
        return 0.3;
    else if (ratio > 1.6 && ratio <= 1.8)
        return 0.4;
    else if (ratio > 1.8 && ratio <= 2.0)
        return 0.6;
    else if (ratio > 2.0 && ratio <= 2.2)
        return 0.9;
    else
        return 1.0;
}














int get_co_ppm(float voltage)
{
  // Stałe wartości dla obliczeń
  const float R0 = 90.26f; // Wartość R0 w kΩ
  const float RL = 4.7f;   // Wartość rezystora pull-down w kΩ

  // Oblicz rezystancję czujnika Rs
  float Rs = RL * (3.3f / voltage - 1.0f);

  // Oblicz stosunek Rs/R0
  float ratio = Rs / R0;

  // Zwróć odpowiednią wartość ppm na podstawie stosunku
  if (ratio > 0.8f)
    return 5;
  else if (ratio > 0.4f)
    return 10;
  else if (ratio > 0.3f)
    return 20;
  else if (ratio > 0.21f)
    return 50;
  else if (ratio > 0.17f)
    return 100;
  else if (ratio > 0.15f)
    return 150;
  else if (ratio > 0.12f)
    return 500;
  else if (ratio > 0.1f)
    return 1000;
  else
    return 5000;
}
float get_hcho_ppm(float vs)
{
    // Assume V0 is a baseline sensor voltage in clean air for HCHO
    const float v0 = 0.5f; // You can adjust this baseline voltage as per your sensor specifications

    // Calculate the ratio Vs/V0
    float ratio = vs / v0;

    // Determine ppm based on the ratio
    if (ratio <= 1.2)
        return 0.1; // Less than 0.1 ppm
    else if (ratio > 1.2 && ratio <= 1.4)
        return 0.2; // Less than 0.2 ppm
    else if (ratio > 1.4 && ratio <= 1.6)
        return 0.3; // Less than 0.3 ppm
    else if (ratio > 1.6 && ratio <= 1.8)
        return 0.4; // Less than 0.4 ppm
    else if (ratio > 1.8 && ratio <= 2.0)
        return 0.6; // Less than 0.6 ppm
    else if (ratio > 2.0 && ratio <= 2.2)
        return 0.9; // Less than 0.9 ppm
    else
        return 1.0; // 1.0 ppm or more
}



// Delay function for BME280
void user_delay_ms(uint32_t period)
{
  HAL_Delay(period);
}

// Function to read and print BME280 sensor data
void print_sensor_data_bme280(struct bme280_t *bme280)
{
  int32_t temp_raw, pressure_raw, humidity_raw;

  // Set the sensor to FORCED_MODE for a single measurement
  bme280_set_power_mode(BME280_FORCED_MODE);
  user_delay_ms(100); // Add delay after setting the mode

  if (bme280_read_uncomp_pressure_temperature_humidity(&pressure_raw, &temp_raw, &humidity_raw) == BME280_OK)
  {
    int32_t v_comp_temp_s32;
    uint32_t v_comp_press_u32;
    uint32_t v_comp_humidity_u32;

    v_comp_temp_s32 = bme280_compensate_temperature_int32(temp_raw);
    v_comp_press_u32 = bme280_compensate_pressure_int32(pressure_raw);
    v_comp_humidity_u32 = bme280_compensate_humidity_int32(humidity_raw);

    float imp_temp = ((float)v_comp_temp_s32 / 100);
    float imp_press = ((float)v_comp_press_u32 / 100);
    float imp_humi = ((float)v_comp_humidity_u32 / 1024);
    float dewpt = imp_temp - ((100 - imp_humi) / 5.0);

    char raw_buffer[200];
    const char separator[] = "________________________________________________________________________\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)separator, strlen(separator), HAL_MAX_DELAY);
    int length = snprintf(raw_buffer, sizeof(raw_buffer), "ENS160_Status: %d, Raw Temp: %ld, Raw Pressure: %ld, Raw Humidity: %ld\r\n",
                          DFRobot_ENS160_GetStatus(&ens160), temp_raw, pressure_raw, humidity_raw);
    HAL_UART_Transmit(&huart2, (uint8_t *)raw_buffer, length, HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2, (uint8_t *)separator, strlen(separator), HAL_MAX_DELAY);

    char display_buffer[200];
    length = snprintf(display_buffer, sizeof(display_buffer), "Temp: %.2f °C, Press: %.2f hPa, Hum: %.2f%% rH, Dew Point: %.2f °C\r\n",
                      imp_temp, imp_press, imp_humi, dewpt);
    HAL_UART_Transmit(&huart2, (uint8_t *)display_buffer, length, HAL_MAX_DELAY);
  }
  else
  {
    const char error_message[] = "Sensor read error!\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)error_message, strlen(error_message), HAL_MAX_DELAY);
  }










  
}


void init_ens160(void)
{
  DFRobot_ENS160_I2C_Init(&ens160, &hi2c1, 0x53);

  while (DFRobot_ENS160_I2C_Begin(&ens160) != NO_ERR)
  {
    printf("ENS160 initialization failed!\r\n");
    HAL_Delay(3000);
  }
  printf("ENS160 initialized successfully!\r\n");

  DFRobot_ENS160_SetPWRMode(&ens160, ENS160_STANDARD_MODE);
  DFRobot_ENS160_SetTempAndHum(&ens160, 25.0, 50.0);
}

// I2C read function for BME280
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint8_t len)
{
  HAL_StatusTypeDef status;

  if (HAL_I2C_IsDeviceReady(&hi2c1, dev_id << 1, 10, 1000) != HAL_OK)
  {
    printf("I2C device not ready!\r\n");
    return -1;
  }

  status = HAL_I2C_Mem_Read(&hi2c1, dev_id << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000);
  if (status != HAL_OK)
  {
    printf("I2C read error: %d\r\n", status);
    return -1;
  }
  return 0;
}

// I2C write function for BME280
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint8_t len)
{
  HAL_StatusTypeDef status;

  if (HAL_I2C_IsDeviceReady(&hi2c1, dev_id << 1, 10, 1000) != HAL_OK)
  {
    printf("I2C device not ready!\r\n");
    return -1;
  }

  status = HAL_I2C_Mem_Write(&hi2c1, dev_id << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000);
  if (status != HAL_OK)
  {
    printf("I2C write error: %d\r\n", status);
    return -1;
  }
  return 0;
}





int process_co_measurement(void)
{
    uint32_t adc_value;
    float co_voltage;

    // Start ADC conversion for CO
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_value = HAL_ADC_GetValue(&hadc1);

    // Convert ADC value to voltage
    co_voltage = 3.3f * adc_value / 4095.0f;

    // Calculate CO ppm
    int co_ppm = calculate_co_ppm(co_voltage);

    return co_ppm;
}

float process_hcho_measurement(void)
{
    uint32_t adc_value;
    float hcho_voltage;

    // Start ADC conversion for HCHO
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_value = HAL_ADC_GetValue(&hadc1);

    // Convert ADC value to voltage
    hcho_voltage = 3.3f * adc_value / 4095.0f;

    // Calculate HCHO ppm
    float hcho_ppm = calculate_hcho_ppm(hcho_voltage);

    return hcho_ppm;
}
















/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */
  // Initialize the ENS160 sensor

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_FATFS_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */


  






  /* E-paper display setup */
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

  if (Epd_Init(&epd) != 0)
  {
    printf("e-Paper init failed\n");
    return 1;
  }

  Epd_Clear(&epd);
  printf("e-Paper init succeed!\n");

  unsigned char top_menu[(400 * 300) / 8 + 100] = {0}; // Bufor dla całego gornego  paska

  // width should be the multiple of 8
  Paint_Init(&paint, top_menu, 400, 300);
  Paint_Clear(&paint, UNCOLORED);

  // Początkowe wypełnienie ekranu

  Epd_DisplayFull(&epd, Paint_GetImage(&paint));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  int thickness = 2; // Grubość obiektów interfejsu

  // Parametry obiektów
  int width = 120;
  int height = 90;

  // Obliczenia przesunięć
  int offset_x = 260; // Przesunięcie poziome
  int offset_y = 100; // Przesunięcie pionowe

  // Obliczanie środka ekranu
  int screen_center_x = EPD_WIDTH / 2+8;
  int screen_center_y = EPD_HEIGHT / 2+8;

  int vertical_gap = 10; // Odległość pionowa między ćwiartkami

  // Obliczanie pozycji ćwiartek względem środka ekranu
  int x0_left = screen_center_x - offset_x / 2 - width / 2;
  int x0_right = screen_center_x + offset_x / 2 - width / 2;
  int y0_top = screen_center_y - offset_y / 2 - height / 2 - 10 - vertical_gap / 2;
  int y0_bottom = screen_center_y + offset_y / 2 - height / 2 - 10 - vertical_gap / 2;

  init_ens160();

  p_bme280->bus_write = user_i2c_write;
  p_bme280->bus_read = user_i2c_read;
  p_bme280->delay_msec = user_delay_ms;
  p_bme280->dev_addr = BME280_I2C_ADDR;

  if (bme280_init(p_bme280) != BME280_OK)
  {
    printf("BME280 initialization failed!\r\n");
    Error_Handler();
  }
  else
  {
    printf("BME280 initialized successfully!\r\n");
  }

  // Configure BME280 sensor
  bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
  bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
  bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
  bme280_set_filter(BME280_FILTER_COEFF_16);
  bme280_set_standby_durn(BME280_STANDBY_TIME_63_MS);




  // Prepare CSV data
    char data_buffer[512];
    snprintf(data_buffer, sizeof(data_buffer), "%ld\r\n", measurement_number++);




  process_SD_card(data_buffer);
 

while (1) {





  
    if (!inMenu) {






CheckAndRestoreCS2();






        // Pobranie aktualnego czasu z RTC
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
        printf("RTC: %02d:%02d:%02d\n", time.Hours, time.Minutes, time.Seconds);

        // Odczyt danych z czujników
        read_and_print_ens160_data();
        print_sensor_data_bme280(p_bme280);
        read_adc_values();

        // Aktualizacja ikony na podstawie wartości enkodera
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int iconIndex = getIconIndex(encoderValue);
        currentIconIndex = iconIndex; // Aktualizacja globalnej zmiennej

        // Czyszczenie ekranu przed rysowaniem
        Paint_Clear(&paint, UNCOLORED);

        // Wyświetlanie sekcji górnej z danymi RTC i wskaźnikiem baterii
        DisplayTopSection(&paint, iconIndex, encoderValue, counter++, batteryLevel);

        // Odczyt poziomu baterii i jego wyświetlenie
        uint8_t bat_percentage = read_soc(&hi2c1);



        batteryLevel = bat_percentage;





// Parametry rysowania
int battery_x = 350;
int battery_y = 3;
int battery_width = 41;
int battery_height = 23;

// Narysuj ikonę baterii
DrawBattery(&paint, battery_x, battery_y, battery_width, battery_height, batteryLevel, COLORED);



char buffer_bat_percentage[20];
snprintf(buffer_bat_percentage, sizeof(buffer_bat_percentage), "%d", bat_percentage);

// Oblicz pozycję do rysowania, aby wycentrować tekst w ikonie baterii
int text_width = strlen(buffer_bat_percentage) * Font20.Width;
int x_center = battery_x + (battery_width - text_width) / 2;
int y_center = battery_y + (battery_height - Font20.Height) / 2+4;

// Rysuj obrys tekstu procentowego
Paint_DrawStringWithOutline(&paint, x_center, y_center, buffer_bat_percentage, &Font20, 2);

// Oblicz pozycję dla "%", tuż po wartości procentowej
x_center += text_width; // Przesuń o szerokość wartości liczbowej

// Rysuj obrys "%" mniejszą czcionką
//Paint_DrawStringWithOutline(&paint, x_center, y_center + 2, "%", &Font16, 1);






        // Rysowanie obiektów na ekranie
        Paint_Universal_Ring(&paint, x0_left, y0_top, width, height, thickness, COLORED, 1); // Ćwiartka: 1
        Paint_Universal_Ring(&paint, x0_right, y0_top, width, height, thickness, COLORED, 2); // Ćwiartka: 2





   Paint_DrawStringAt(&paint, 306, 80, "TVOC", &Font20, COLORED);
  Paint_DrawStringAt(&paint, 24, 80, "CO2", &Font20, COLORED);








// Wyświetlanie danych TVOC z dołączonym "ppm" mniejszą czcionką






uint8_t tvoc = DFRobot_ENS160_GetTVOC(&ens160);
char buffer_tvoc[20];
snprintf(buffer_tvoc, sizeof(buffer_tvoc), "%d", tvoc);

int x_pos_tvoc = 308;
int y_pos_tvoc = 100;

// Rysowanie wartości TVOC większą czcionką
Paint_DrawStringAt(&paint, x_pos_tvoc, y_pos_tvoc, buffer_tvoc, &Font20, COLORED);

// Przesunięcie x_pos_tvoc o szerokość narysowanego tekstu wartości TVOC
x_pos_tvoc += strlen(buffer_tvoc) * Font20.Width; 

// Rysowanie "ppm" mniejszą czcionką, zaraz po wartości
Paint_DrawStringAt(&paint, x_pos_tvoc + 2, y_pos_tvoc + 4, "ppm", &Font16, COLORED);

Paint_Universal_Ring(&paint, x0_left, y0_bottom, width, height, thickness, COLORED, 3); // Ćwiartka: 3







int x_pos_co2 = 24;
int y_pos_co2 = 100;


 



// Wyświetlanie danych CO2 z dołączonym "ppm" mniejszą czcionką
uint16_t co2 = DFRobot_ENS160_GetECO2(&ens160);
char buffer_CO2[20];
snprintf(buffer_CO2, sizeof(buffer_CO2), "%d", co2);






// Rysowanie wartości CO2 większą czcionką
Paint_DrawStringAt(&paint, x_pos_co2, y_pos_co2, buffer_CO2, &Font20, COLORED);

// Przesunięcie x_pos_co2 o szerokość narysowanego tekstu wartości CO2
x_pos_co2 += strlen(buffer_CO2) * Font20.Width; 

// Rysowanie "ppm" mniejszą czcionką, zaraz po wartości
Paint_DrawStringAt(&paint, x_pos_co2 + 2, y_pos_co2 + 4, "ppm", &Font16, COLORED);











        Paint_Universal_Ring(&paint, x0_right, y0_bottom, width, height, thickness, COLORED, 4); // Ćwiartka: 4


        // Rysowanie centralnego pierścienia z AQI
        int center_x = (x0_left + width / 2 + x0_right + width / 2) / 2;
        int center_y = (y0_top + height / 2 + y0_bottom + height / 2) / 2;
        int outer_radius = screen_center_x - x0_left - height - vertical_gap - thickness;
        Paint_DrawRing(&paint, center_x, center_y, outer_radius, thickness, COLORED);

        uint8_t aqi = DFRobot_ENS160_GetAQI(&ens160);
        char buffer_AQI[100];
        snprintf(buffer_AQI, sizeof(buffer_AQI), "%d", aqi);
        Paint_DrawStringAtCenter(&paint, center_y, buffer_AQI, &Font20, 400);
        Paint_DrawStringAtCenter(&paint, center_y - 20, "AQI:", &Font20, 400);




int co_ppm = process_co_measurement();
    float hcho_ppm = process_hcho_measurement();





    Paint_DrawStringAt(&paint, 22, 180 , "CO", &Font20, COLORED);
 Paint_DrawStringAt(&paint, 306, 180 , "HCHO", &Font20, COLORED);
 



// Deklaracja zmiennych dla bufora i pozycji dla HCHO
char buffer_HCHO[10];
int x_pos_hcho = 296;
int y_pos_hcho = 200;

// Rysowanie "<" i wartości hcho_ppm
snprintf(buffer_HCHO, sizeof(buffer_HCHO), "<%.1f", hcho_ppm);
Paint_DrawStringAt(&paint, x_pos_hcho, y_pos_hcho, buffer_HCHO, &Font20, COLORED);

// Przesunięcie x_pos_hcho o szerokość narysowanego tekstu "<0.5"
x_pos_hcho += strlen(buffer_HCHO) * Font20.Width;

// Rysowanie "ppm" mniejszą czcionką, zaraz po wartości
Paint_DrawStringAt(&paint, x_pos_hcho + 5, y_pos_hcho + 2, "ppm", &Font16, COLORED);



// Deklaracja zmiennych dla bufora i pozycji
char buffer_CO[10];
int x_pos = 22;
int y_pos = 200;

// Rysowanie "<" i wartości co_ppm
snprintf(buffer_CO, sizeof(buffer_CO), "<%d", co_ppm);
Paint_DrawStringAt(&paint, x_pos, y_pos, buffer_CO, &Font20, COLORED);

// Przesunięcie x_pos o szerokość narysowanego tekstu "<100"
x_pos += strlen(buffer_CO) * Font20.Width; 

// Rysowanie "ppm" mniejszą czcionką, zaraz po wartości
Paint_DrawStringAt(&paint, x_pos + 5, y_pos + 2, "ppm", &Font16, COLORED);













        // Odczyt i wyświetlanie temperatury oraz wilgotności z BME280
        int32_t temp_raw = 0, pressure_raw = 0, humidity_raw = 0;
        if (bme280_read_uncomp_pressure_temperature_humidity(&pressure_raw, &temp_raw, &humidity_raw) == BME280_OK) {
            int32_t bme280_temp = bme280_compensate_temperature_int32(temp_raw);
            float temperature_celsius = bme280_temp / 100.0;

            uint32_t bme280_humidity = bme280_compensate_humidity_int32(humidity_raw);
            float humidity_percentage = bme280_humidity / 1024.0;

            char buffer_TEMP[50], buffer_HUM[50];
            snprintf(buffer_TEMP, sizeof(buffer_TEMP), "%.1f", temperature_celsius);
            snprintf(buffer_HUM, sizeof(buffer_HUM), "%.1f", humidity_percentage);

            int line_y = center_y + 40;
            Paint_DrawStringAt(&paint, center_x - 60, line_y - 20, "T:", &Font16, 400);
            Paint_DrawStringAt(&paint, center_x - 60, line_y, buffer_TEMP, &Font16, 400);
            Paint_DrawStringAt(&paint, center_x + 20, line_y - 20, "H:", &Font16, 400);
            Paint_DrawStringAt(&paint, center_x + 20, line_y, buffer_HUM, &Font16, 400);
        } else {
            Paint_DrawStringAtCenter(&paint, center_y + 40, "Error", &Font16, 400);
        }

        // Wyświetlanie dolnej sekcji z ikonami







int height = 40;     // Wysokość prostokątów
int distance = 10;   // Odległość między prostokątami
int y0_bottom = EPD_HEIGHT - height - 10; // Pozycja Y dolnego prostokąta, blisko dolnej krawędzi ekranu





//Paint_Draw3RectanglesCenter(&paint, y0_bottom, height, distance, thickness, COLORED, x0_left, EPD_WIDTH-2);




        DisplayBottomSection(&paint, iconIndex);



   

if ( sdNotOk==1    &&   (loopCounter % 2 == 0)   && (CheckAndRestoreCS2() == 1   )       ) {
    Paint_DrawBitmap(&paint, icon_exclamation_mark, 286, 0, 32, 32, COLORED); 
}





/* 

if ( sdNotOk==1      ) {
    Paint_DrawBitmap(&paint, icon_exclamation_mark, 286, 0, 32, 32, COLORED); 
}


 */



        
        Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);



  










        HAL_Delay(10);


// Pełne odświeżenie ekranu co 180 cykli
loopCounter++;
if (loopCounter >= 205) {
    Epd_Clear(&epd);
    Paint_Clear(&paint, UNCOLORED);
    Epd_DisplayFull(&epd, Paint_GetImage(&paint));
    loopCounter = 0;
}

// Proces SD co 10 cykli
if (loopCounter % 10 == 0) {
    process_SD_card(data_buffer);
}










    }
















    // Sprawdzenie, czy należy przejść do menu
    if (GoToMenu > 0 && GoToMenu <= 8) {
        inMenu = 1;  // Ustawienie flagi przed wejściem do menu
        switch (GoToMenu) {
            case 1:
                ShowMenu1();
                break;
            case 2:
                ShowMenu2();
                break;
            case 3:
                ShowMenu3();
                break;
            case 4:
                ShowMenu4();
                break;
            case 5:
                ShowMenu5();
                break;
            case 6:
                ShowMenu6();
                break;
            case 7:
                ShowMenu7();
                break;
            case 8:
                ShowMenu8();
                break;
        }
        GoToMenu = 0; // Zresetowanie zmiennej po obsłużeniu
        inMenu = 0;  // Powrót do normalnej pracy po wyjściu z menu
    }
}


  /* USER CODE END WHILE */
















  /* USER CODE BEGIN 3 */

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
   */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
   */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
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
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
   */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
   */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
   */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 29;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DC_Pin | RST_Pin | CS_Pin | CS_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DC_Pin RST_Pin CS_Pin CS_2_Pin */
  GPIO_InitStruct.Pin = DC_Pin | RST_Pin | CS_Pin | CS_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUSY_Pin */
  GPIO_InitStruct.Pin = BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZ_Pin */
  GPIO_InitStruct.Pin = BUZZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EN_SW_Pin */
  GPIO_InitStruct.Pin = EN_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(EN_SW_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == EN_SW_Pin)
  {
    uint32_t current_time = HAL_GetTick();
    if ((current_time - last_interrupt_time) > debounce_time)
    {
      last_interrupt_time = current_time;

      if (inMenu)
      {
        // Jeśli encoderPosition jest w zakresie 1-5, to ustawiamy wartości, a nie wychodzimy z menu
        if (encoderPosition > 0 && encoderPosition <= 5)
        {
          EditMenu3Setting();
        }
        else if (canExitMenu == NULL || canExitMenu())
        {
          inMenu = 0; // Wyjście z menu tylko, gdy encoderPosition == 0
            __HAL_TIM_SET_COUNTER(&htim2, 2); 
            
        }
      }
      else
      {
        // Wejdź do menu
        printf("Przycisk enkodera wciśnięty! Aktualna ikona: %d\n", currentIconIndex);

        // Ustawienie GoToMenu na podstawie currentIconIndex
        GoToMenu = currentIconIndex;

        inMenu = 1; // Ustawienie flagi inMenu na true
      }

      // Włączenie BUZZ
      HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);

      // Uruchomienie timera, aby wyłączyć BUZZ po 100 ms
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      HAL_TIM_Base_Start_IT(&htim3);
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);
    HAL_TIM_Base_Stop_IT(htim);
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
