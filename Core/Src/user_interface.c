#include "user_interface.h"
#include "epd4in2b.h"
#include "fonts.h"
#include "imagedata.h"
#include <stdio.h>
#include "globals.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "epdpaint.h"


int encoderPosition = 0; // Zmienna globalna, inicjalizowana przy starcie


int buttonState = 0;
int lastButtonState = 0;
uint32_t lastDebounceTime = 0;

int currentDisplayMode = COLORED;

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;
 extern volatile bool sdNotOk;




bool isEditing = false; 


extern Paint paint;
extern Epd epd;

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim2;


extern int (*canExitMenu)(void);


void ShowMenu1(void)
{
    
    printf("1 przycisk jest wcisniety!\n");

    // Inicjalizacja licznika
    int counter = 1;
    char buffer[50]; // Bufor na tekst do wyświetlenia

    // Pętla while, która będzie działać dopóki jesteśmy w menu
    while (inMenu)
    {
        // Czyszczenie ekranu
        Paint_Clear(&paint, UNCOLORED);

        // Generowanie tekstu z aktualną wartością licznika
        snprintf(buffer, sizeof(buffer), "Counter: %d", counter);

        // Wyświetlanie licznika na ekranie
        Paint_DrawStringAtCenter(&paint, 50, buffer, &Font20, 400);

        Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

        // Zwiększenie licznika
        counter++;

        // Opóźnienie, aby zmiany były widoczne
        HAL_Delay(1000);
    }
}

void ShowMenu2(void)
{
    while (inMenu)
    {

        HAL_Delay(200);
    }
}


void ShowMenu3(void)
{
    printf("3 przycisk jest wcisniety!\n");

    encoderPosition = 0; // Ustaw wartość na 0 przy wejściu do funkcji
    int blinkCounter = 0; // Dodaj licznik migania

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej

    while (inMenu)
    {
        if (!isEditing) // Jeśli nie jesteśmy w trybie edycji
        {
            Paint_Clear(&paint, UNCOLORED);

            uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
            int encoderDirection = encoderValue > previousEncoderValue ? 1 : (encoderValue < previousEncoderValue ? -1 : 0);
            previousEncoderValue = encoderValue;

            // Aktualizuj encoderPosition z zawijaniem
            if (encoderDirection != 0) {
                encoderPosition += encoderDirection;

                if (encoderPosition < 1) {
                    encoderPosition = 5; // Zawijanie z 1 na 5
                } else if (encoderPosition > 5) {
                    encoderPosition = 1; // Zawijanie z 5 na 1
                }

                // Zresetuj licznik migania przy zmianie pozycji
                blinkCounter = 0;
            }

            // Pobierz aktualny czas i datę z RTC
            RTC_TimeTypeDef currentTime;
            RTC_DateTypeDef currentDate;
            HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);


            Paint_DrawStringAtCenter(&paint, 50, "USTAW GODZINE", &Font20, 400);

            char buffer_hour[10];
            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);

            char buffer_minute[10];
            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);

            Paint_DrawStringAtCenter(&paint, 150, "USTAW DATE", &Font20, 400);


            char buffer_day[10];
            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);


            char buffer_month[10];
            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);

            char buffer_year[10];
            snprintf(buffer_year, sizeof(buffer_year), "%d", 2000 + currentDate.Year);
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            HAL_Delay(50);

            // Miganie wybranej pozycji
            switch (encoderPosition)
            {
                case 0:
                    break;
                case 1:
                    Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, UNCOLORED);
                    break;
                case 2:
                    Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, UNCOLORED);
                    break;
                case 3:
                    Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, UNCOLORED);
                    break;
                case 4:
                    Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, UNCOLORED);
                    break;
                case 5:
                    Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, UNCOLORED);
                    break;
                default:
                    break;
            }

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            // Zliczaj iteracje migania
            if (encoderPosition != 0) {
                blinkCounter++;
            }

            if (blinkCounter >= 4) {
                encoderPosition = 0;
                blinkCounter = 0;
            }
        }
        else
        {
            // Jeśli jesteśmy w trybie edycji, czekamy na zakończenie edycji
            switch (encoderPosition)
            {
                case 0:
                    isEditing = 0;
                    break;
                case 1:
                    EditHourSetting();
                    break;
                case 2:
                    EditMinuteSetting();
                    break;
                case 3:
                    EditDaySetting();
                    break;
                case 4:
                    EditMonthSetting();
                    break;
                case 5:
                    EditYearSetting();
                    break;
                default:
                    isEditing = 0;
                    break;
            }
        }
    }
}



int CanExitMenu3(void)
{
    return (encoderPosition == 0);

}



void EditMenu3Setting()

{
    isEditing=1;
   
}

void EditHourSetting(void)
{
    printf("Edycja godzin\n");

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej
    RTC_TimeTypeDef currentTime;
    RTC_DateTypeDef currentDate;
    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN); // Pobranie aktualnej godziny i minut
    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN); // Pobranie aktualnej daty
    int currentHour = currentTime.Hours;  // Inicjalizacja godziny

    // Wyświetl początkową godzinę
    char buffer_hour[10];
    snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
    Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int scaledChange = 0;     // Zmiana skalowana (nagromadzona zmiana)

    while (isEditing) {
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int delta = (int)(encoderValue - previousEncoderValue); // Zmiana wartości enkodera
        previousEncoderValue = encoderValue;

        // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
        if (abs(delta) > 12)
        {   
            lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
            continue; // Ignorujemy nagłe skoki (teleportacje)
        }

        scaledChange += delta;

        if (scaledChange >= 1 || scaledChange <= -1) {
            // Aktualizuj currentHour w zależności od zmiany wartości enkodera
            currentHour += scaledChange;

            // Zawijanie wartości godziny w zakresie 0-23
            if (currentHour < 0) {
                currentHour = 23;
            } else if (currentHour > 23) {
                currentHour = 0;
            }

            // Wyświetl aktualną wartość godziny po zmianie
            printf("Aktualna godzina: %02d\n", currentHour);

            // Aktualizuj wyświetlacz
            Paint_Clear(&paint, UNCOLORED);

            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentHour);
            char buffer_minute[10];
            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
            char buffer_day[10];
            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
            char buffer_month[10];
            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));
            char buffer_year[10];
            snprintf(buffer_year, sizeof(buffer_year), "%d", 2000 + currentDate.Year);

            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            Paint_DrawStringAtCenter(&paint, 50, "USTAW GODZINE", &Font20, 400);
            Paint_DrawStringAtCenter(&paint, 150, "USTAW DATE", &Font20, 400);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            scaledChange = 0; // Zresetowanie skali po aktualizacji
        }

        HAL_Delay(100); // Zwiększenie opóźnienia dla wyraźnego migania

        // Sprawdzenie, czy przycisk został naciśnięty, aby zakończyć edycję
        if (HAL_GPIO_ReadPin(EN_SW_GPIO_Port, EN_SW_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(50); // Debouncing
            printf("KLIK!\n");
            isEditing = 0;  // Zakończ edycję

            // Zapisz nową godzinę w systemie
            RTC_TimeTypeDef new_time = currentTime;
            new_time.Hours = currentHour;
            HAL_RTC_SetTime(&hrtc, &new_time, RTC_FORMAT_BIN); // Zapisz nową godzinę
            printf("Nowa godzina ustawiona na: %02d\n", currentHour);
        }
    }
}




void EditMinuteSetting(void)
{
    printf("Edycja minut\n");

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej
    RTC_TimeTypeDef currentTime;
    RTC_DateTypeDef currentDate;
    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN); // Pobranie aktualnych minut i godzin
    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN); // Pobranie aktualnej daty
    int currentMinute = currentTime.Minutes;  // Inicjalizacja minut

    // Wyświetl początkowe minuty
    char buffer_minute[10];
    snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
    Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int scaledChange = 0;     // Zmiana skalowana (nagromadzona zmiana)

    while (isEditing) {
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int delta = (int)(encoderValue - previousEncoderValue); // Zmiana wartości enkodera
        previousEncoderValue = encoderValue;

        // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
        if (abs(delta) > 15)
        {   
            lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
            continue; // Ignorujemy nagłe skoki (teleportacje)
        }

        scaledChange += delta;

        if (scaledChange >= 1 || scaledChange <= -1) {
            // Aktualizuj currentMinute w zależności od zmiany wartości enkodera
            currentMinute += scaledChange;

            // Zawijanie wartości minut w zakresie 0-59
            if (currentMinute < 0) {
                currentMinute = 59;
            } else if (currentMinute > 59) {
                currentMinute = 0;
            }

            // Wyświetl aktualną wartość minut po zmianie
            printf("Aktualna minuta: %02d\n", currentMinute);

            // Aktualizuj wyświetlacz
            Paint_Clear(&paint, UNCOLORED);

            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentMinute);
            char buffer_hour[10];
            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
            char buffer_day[10];
            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
            char buffer_month[10];
            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));
            char buffer_year[10];
            snprintf(buffer_year, sizeof(buffer_year), "%d", 2000 + currentDate.Year);

            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            Paint_DrawStringAtCenter(&paint, 50, "USTAW MINUTY", &Font20, 400);
            Paint_DrawStringAtCenter(&paint, 150, "USTAW DATE", &Font20, 400);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            scaledChange = 0; // Zresetowanie skali po aktualizacji
        }

        HAL_Delay(100); // Zwiększenie opóźnienia dla wyraźnego migania

        // Sprawdzenie, czy przycisk został naciśnięty, aby zakończyć edycję
        if (HAL_GPIO_ReadPin(EN_SW_GPIO_Port, EN_SW_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(50); // Debouncing
            printf("KLIK!\n");
            isEditing = 0;  // Zakończ edycję

            // Zapisz nowe minuty w systemie, zerując sekundy
            RTC_TimeTypeDef new_time = currentTime;
            new_time.Minutes = currentMinute;
            new_time.Seconds = 0; // Zerowanie sekund
            HAL_RTC_SetTime(&hrtc, &new_time, RTC_FORMAT_BIN); // Zapisz nowe minuty
            printf("Nowa minuta ustawiona na: %02d\n", currentMinute);
        }
    }
}








void EditDaySetting(void)
{
    printf("Edycja dnia\n");

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej
    RTC_DateTypeDef currentDate;
    RTC_TimeTypeDef currentTime;

    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN); // Pobranie aktualnej daty
    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN); // Pobranie aktualnego czasu
    int currentDay = currentDate.Date;  // Inicjalizacja dnia

    // Pobierz maksymalną liczbę dni dla bieżącego miesiąca
    int maxDay = 31;
    if (currentDate.Month == 2) {
        // Luty, sprawdzamy, czy rok jest przestępny
        maxDay = (currentDate.Year % 4 == 0 && (currentDate.Year % 100 != 0 || currentDate.Year % 400 == 0)) ? 29 : 28;
    } else if (currentDate.Month == 4 || currentDate.Month == 6 || currentDate.Month == 9 || currentDate.Month == 11) {
        // Kwiecień, czerwiec, wrzesień, listopad
        maxDay = 30;
    }

    char buffer_day[10];
    snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
    Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int scaledChange = 0;     // Zmiana skalowana (nagromadzona zmiana)

    while (isEditing) {
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int delta = (int)(encoderValue - previousEncoderValue); // Zmiana wartości enkodera
        previousEncoderValue = encoderValue;

        // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
        if (abs(delta) > 12)
        {   
            lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
            continue; // Ignorujemy nagłe skoki (teleportacje)
        }

        scaledChange += delta;

        if (scaledChange >= 1 || scaledChange <= -1) {
            // Aktualizuj currentDay w zależności od zmiany wartości enkodera
            currentDay += scaledChange;

            // Zawijanie wartości dnia w zakresie 1-maxDay
            if (currentDay < 1) {
                currentDay = maxDay;
            } else if (currentDay > maxDay) {
                currentDay = 1;
            }

            // Wyświetl aktualną wartość dnia po zmianie
            printf("Aktualny dzień: %02d\n", currentDay);

            // Aktualizuj wyświetlacz
            Paint_Clear(&paint, UNCOLORED);

            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDay);

            // Wyświetlanie zaktualizowanego dnia
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);

            // Wyświetlanie pozostałych elementów
            char buffer_hour[10];
            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
            char buffer_minute[10];
            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
            char buffer_month[10];
            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));
            char buffer_year[10];
            snprintf(buffer_year, sizeof(buffer_year), "%d", 2000 + currentDate.Year);

            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            Paint_DrawStringAtCenter(&paint, 50, "USTAW GODZINE", &Font20, 400);
            Paint_DrawStringAtCenter(&paint, 150, "USTAW DZIEN", &Font20, 400);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            scaledChange = 0; // Zresetowanie skali po aktualizacji
        }

        HAL_Delay(100); // Opóźnienie

        // Sprawdzenie, czy przycisk został naciśnięty, aby zakończyć edycję
        if (HAL_GPIO_ReadPin(EN_SW_GPIO_Port, EN_SW_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(50); // Debouncing
            printf("KLIK!\n");
            isEditing = 0;  // Zakończ edycję

            // Zapisz nowy dzień w systemie
            RTC_DateTypeDef new_date = {0};
            HAL_RTC_GetDate(&hrtc, &new_date, RTC_FORMAT_BIN);
            new_date.Date = currentDay;
            HAL_RTC_SetDate(&hrtc, &new_date, RTC_FORMAT_BIN); // Zapisz nowy dzień
            printf("Nowy dzień ustawiony na: %02d\n", currentDay);
        }
    }
}


void EditMonthSetting(void)
{
    printf("Edycja miesiąca\n");

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej
    RTC_DateTypeDef currentDate;
    RTC_TimeTypeDef currentTime;

    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN); // Pobranie aktualnej daty
    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN); // Pobranie aktualnego czasu
    int currentMonth = currentDate.Month;  // Inicjalizacja miesiąca

    char buffer_month[10];
    snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));
    Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);

    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int scaledChange = 0;     // Zmiana skalowana (nagromadzona zmiana)

    while (isEditing) {
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int delta = (int)(encoderValue - previousEncoderValue); // Zmiana wartości enkodera
        previousEncoderValue = encoderValue;

        // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
        if (abs(delta) > 12)
        {   
            lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
            continue; // Ignorujemy nagłe skoki (teleportacje)
        }

        scaledChange += delta;

        if (scaledChange >= 1 || scaledChange <= -1) {
            // Aktualizuj currentMonth w zależności od zmiany wartości enkodera
            currentMonth += scaledChange;

            // Zawijanie wartości miesiąca w zakresie 1-12
            if (currentMonth < 1) {
                currentMonth = 12;
            } else if (currentMonth > 12) {
                currentMonth = 1;
            }

            // Wyświetl aktualną wartość miesiąca po zmianie
            printf("Aktualny miesiąc: %02d\n", currentMonth);

            // Aktualizuj wyświetlacz
            Paint_Clear(&paint, UNCOLORED);

            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentMonth));

            // Wyświetlanie zaktualizowanego miesiąca
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);

            // Wyświetlanie pozostałych elementów
            char buffer_hour[10];
            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
            char buffer_minute[10];
            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
            char buffer_day[10];
            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
            char buffer_year[10];
            snprintf(buffer_year, sizeof(buffer_year), "%d", 2000 + currentDate.Year);

            Paint_DrawStringAtCenter(&paint, 50, "USTAW GODZINE", &Font20, 400);
            Paint_DrawStringAtCenter(&paint, 150, "USTAW MIESIAC", &Font20, 400);

            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            scaledChange = 0; // Zresetowanie skali po aktualizacji
        }

        HAL_Delay(100); // Opóźnienie

        // Sprawdzenie, czy przycisk został naciśnięty, aby zakończyć edycję
        if (HAL_GPIO_ReadPin(EN_SW_GPIO_Port, EN_SW_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(50); // Debouncing
            printf("KLIK!\n");
            isEditing = 0;  // Zakończ edycję

            // Zapisz nowy miesiąc w systemie
            RTC_DateTypeDef new_date = {0};
            HAL_RTC_GetDate(&hrtc, &new_date, RTC_FORMAT_BIN);
            new_date.Month = currentMonth;
            HAL_RTC_SetDate(&hrtc, &new_date, RTC_FORMAT_BIN); // Zapisz nowy miesiąc
            printf("Nowy miesiąc ustawiony na: %02d\n", currentMonth);
        }
    }
}






void EditYearSetting(void)
{
    printf("Edycja roku\n");

    uint32_t previousEncoderValue = __HAL_TIM_GET_COUNTER(&htim2); // Inicjalizacja zmiennej
    RTC_DateTypeDef currentDate;
    RTC_TimeTypeDef currentTime;

    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN); // Pobranie aktualnej daty
    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN); // Pobranie aktualnego czasu
    int currentYear = 2000 + currentDate.Year;  // Inicjalizacja roku

    char buffer_year[10];
    snprintf(buffer_year, sizeof(buffer_year), "%d", currentYear);
    Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int scaledChange = 0;     // Zmiana skalowana (nagromadzona zmiana)

    while (isEditing) {
        uint32_t encoderValue = __HAL_TIM_GET_COUNTER(&htim2);
        int delta = (int)(encoderValue - previousEncoderValue); // Zmiana wartości enkodera
        previousEncoderValue = encoderValue;

        // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
        if (abs(delta) > 12)
        {   
            lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
            continue; // Ignorujemy nagłe skoki (teleportacje)
        }

        scaledChange += delta;

        if (scaledChange >= 1 || scaledChange <= -1) {
            // Aktualizuj currentYear w zależności od zmiany wartości enkodera
            currentYear += scaledChange;

            // Zawijanie wartości roku w zakresie 2000-2099
            if (currentYear < 2000) {
                currentYear = 2099;
            } else if (currentYear > 2099) {
                currentYear = 2000;
            }

            // Wyświetl aktualną wartość roku po zmianie
            printf("Aktualny rok: %d\n", currentYear);

            // Aktualizuj wyświetlacz
            Paint_Clear(&paint, UNCOLORED);

            snprintf(buffer_year, sizeof(buffer_year), "%d", currentYear);

            // Wyświetlanie zaktualizowanego roku
            Paint_DrawStringAt(&paint, 200, 200, buffer_year, &Font20, COLORED);

            // Wyświetlanie pozostałych elementów
            char buffer_hour[10];
            snprintf(buffer_hour, sizeof(buffer_hour), "%02d:", currentTime.Hours);
            char buffer_minute[10];
            snprintf(buffer_minute, sizeof(buffer_minute), "%02d", currentTime.Minutes);
            char buffer_day[10];
            snprintf(buffer_day, sizeof(buffer_day), "%02d", currentDate.Date);
            char buffer_month[10];
            snprintf(buffer_month, sizeof(buffer_month), "%s", getMonthStr(currentDate.Month));

            Paint_DrawStringAt(&paint, 170, 100, buffer_hour, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 215, 100, buffer_minute, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 120, 200, buffer_day, &Font20, COLORED);
            Paint_DrawStringAt(&paint, 150, 200, buffer_month, &Font20, COLORED);

            Paint_DrawStringAtCenter(&paint, 50, "USTAW GODZINE", &Font20, 400);
            Paint_DrawStringAtCenter(&paint, 150, "USTAW ROK", &Font20, 400);

            Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);

            scaledChange = 0; // Zresetowanie skali po aktualizacji
        }

        HAL_Delay(100); // Opóźnienie

        // Sprawdzenie, czy przycisk został naciśnięty, aby zakończyć edycję
        if (HAL_GPIO_ReadPin(EN_SW_GPIO_Port, EN_SW_Pin) == GPIO_PIN_RESET) {
            HAL_Delay(50); // Debouncing
            printf("KLIK!\n");
            isEditing = 0;  // Zakończ edycję

            // Zapisz nowy rok w systemie
            RTC_DateTypeDef new_date = {0};
            HAL_RTC_GetDate(&hrtc, &new_date, RTC_FORMAT_BIN);
            new_date.Year = currentYear - 2000;
            HAL_RTC_SetDate(&hrtc, &new_date, RTC_FORMAT_BIN); // Zapisz nowy rok
            printf("Nowy rok ustawiony na: %d\n", currentYear);
        }
    }
}
















void ShowMenu4(void)
{
    printf("Wyświetlanie Menu 4\n");
    // Wyświetlenie treści menu 4
    Paint_Clear(&paint, UNCOLORED);
    Paint_DrawStringAtCenter(&paint, EPD_HEIGHT / 2, "Hello from 4 menu!", &Font20, 400);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);
}

void ShowMenu5(void)
{
    printf("Wyświetlanie Menu 5\n");
    // Wyświetlenie treści menu 5
    Paint_Clear(&paint, UNCOLORED);
    Paint_DrawStringAtCenter(&paint, EPD_HEIGHT / 2, "Hello from 5 menu!", &Font20, 400);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);
}

void ShowMenu6(void)
{
    printf("Wyświetlanie Menu 6\n");
    // Wyświetlenie treści menu 6
    Paint_Clear(&paint, UNCOLORED);
    Paint_DrawStringAtCenter(&paint, EPD_HEIGHT / 2, "Hello from 6 menu!", &Font20, 400);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);
}

void ShowMenu7(void)
{
    printf("Wyświetlanie Menu 7\n");
    // Wyświetlenie treści menu 7
    Paint_Clear(&paint, UNCOLORED);
    Paint_DrawStringAtCenter(&paint, EPD_HEIGHT / 2, "Hello from 7 menu!", &Font20, 400);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);
}

void ShowMenu8(void)
{
    printf("Wyświetlanie Menu 8\n");
    // Wyświetlenie treści menu 8
    Paint_Clear(&paint, UNCOLORED);
    Paint_DrawStringAtCenter(&paint, EPD_HEIGHT / 2, "Hello from 8 menu!", &Font20, 400);
    Epd_Display_Partial_DMA(&epd, Paint_GetImage(&paint), 0, 0, 400, 300);
}

void DisplayTopSection(Paint *paint, int iconIndex, uint32_t encoderValue, int counter, uint8_t batteryLevel)
{
    // Pobierz czas i datę
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    // Formatowanie daty i czasu
    char dateStr[30];
    snprintf(dateStr, sizeof(dateStr), "%02d %s", date.Date, getMonthStr(date.Month));

    char timeStr[30];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);

    char yearStr[10];
    snprintf(yearStr, sizeof(yearStr), "%04d", 2000 + date.Year);

    // Wyświetl datę po lewej stronie
    Paint_DrawStringAt(paint, 18, 2, dateStr, &Font20, COLORED);

    // Wyświetl czas na środku na górze
    Paint_DrawStringAtCenter(paint, 10, timeStr, &Font20, 400);

    Paint_DrawStringAt(paint, 18, 20, yearStr, &Font20, COLORED);



if(CheckAndRestoreCS2() == 1) {
    
     Paint_DrawBitmap(paint, icon_sd, 315, 0, 32, 32, COLORED); 
       
     }






}

const char *getMonthStr(uint8_t month)
{
    switch (month)
    {
    case 1:
        return "sty";
    case 2:
        return "lut";
    case 3:
        return "mar";
    case 4:
        return "kwi";
    case 5:
        return "maj";
    case 6:
        return "cze";
    case 7:
        return "lip";
    case 8:
        return "sie";
    case 9:
        return "wrz";
    case 10:
        return "paz";
    case 11:
        return "lis";
    case 12:
        return "gru";
    default:
        return "???";
    }
}

void DisplayMiddleSection(Paint *paint)
{

    // Wywołanie funkcji 10 jednostek poniżej y0_bottom
    // Paint_Draw3RectanglesCenter(&paint, y0_bottom + height + 10, r_height, vertical_gap, thickness, COLORED, x0_left, x0_right + width);

    // Rysowanie linii poziomej
    // Paint_DrawLineWithThickness(&paint, x0_left, y0_bottom + height + 10 + r_height + vertical_gap, x0_right + width, y0_bottom + height + 10 + r_height + vertical_gap, thickness, COLORED);

    // Rysowanie pionowej linii
}

void DisplayBottomSection(Paint *paint, int iconIndex)
{
    int icon_height = 245;
    int desc_offset = 5;
    const char *iconDescriptions[] = {
        "Wykresy",     // Opis dla ikony 1
        "Wilgotnosc",  // Opis dla ikony 2
        "Data i Czas",      // Opis dla ikony 3
        "Lisc",        // Opis dla ikony 4
        "Pomiary",     // Opis dla ikony 5
        "Tryb ciemny", // Opis dla ikony 6
        "Wiatr",       // Opis dla ikony 7
        "Ustawienia"   // Opis dla ikony 8
    };
    // Upewnij się, że indeks jest w zakresie
    if (iconIndex < 1 || iconIndex > 8)
    {
        return;
    }

    // Wyświetlanie opisu ikony nad bitmapą
    Paint_DrawStringAtCenter(paint, icon_height - desc_offset, iconDescriptions[iconIndex - 1], &Font20, 400);
    switch (iconIndex)
    {

    case 1:
        Paint_DrawBitmap(paint, icon_temp, 5, icon_height, 48, 48, COLORED);
        Paint_DrawStringAtCenter(paint, icon_height - desc_offset, "Wykresy", &Font20, 400);
        break;
    case 2:
        Paint_DrawBitmap(paint, icon_humi, 55, icon_height, 48, 48, COLORED);
        break;
    case 3:
        Paint_DrawBitmap(paint, icon_sun, 105, icon_height, 48, 48, COLORED);
        break;
    case 4:
        Paint_DrawBitmap(paint, icon_leaf, 155, icon_height, 48, 48, COLORED);
        break;
    case 5:
        Paint_DrawBitmap(paint, icon_sunset, 205, icon_height, 48, 48, COLORED);
        break;
    case 6:
        Paint_DrawBitmap(paint, icon_sunrise, 255, icon_height, 48, 48, COLORED);
        break;
    case 7:
        Paint_DrawBitmap(paint, icon_wind, 305, icon_height, 48, 48, COLORED);
        break;
    case 8:
        Paint_DrawBitmap(paint, icon_settings, 355, icon_height, 48, 48, COLORED);
        break;
    default:
        break;
    }
}
int getIconIndex(uint32_t encoderValue)
{
    static int lastEncoderValue = 0; // Przechowuje poprzednią wartość enkodera
    static int iconIndex = 1;        // Zakładam, że startujesz od ikony 1

    // Oblicz różnicę wartości enkodera
    int delta = (int)(encoderValue)-lastEncoderValue;

    // Aktualizacja wartości enkodera tylko jeśli zmiana jest w rozsądnym zakresie
    if (abs(delta) > 12)
    {                                           // Ignorowanie dużych zmian (np. teleportacji)
        lastEncoderValue = (int)(encoderValue); // Mimo to aktualizujemy lastEncoderValue
        return iconIndex;
    }

    lastEncoderValue = (int)(encoderValue);

    // Obsługa ruchów
    if (delta > 0)
    { // Ruch w prawo
        iconIndex += delta;
        if (iconIndex > 8)
        {
            iconIndex = 8; // Ogranicz do maksymalnej wartości
        }
    }
    else if (delta < 0)
    { // Ruch w lewo
        iconIndex += delta;
        if (iconIndex < 1)
        {
            iconIndex = 1; // Ogranicz do minimalnej wartości
        }
    }

    return iconIndex;
}

void UpdateBatteryLevel(uint8_t *batteryLevel)
{
    *batteryLevel = (*batteryLevel + 1) % 4;
}