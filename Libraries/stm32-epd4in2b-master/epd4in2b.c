#include "epd4in2b.h"
#include "stm32l4xx_hal.h"

// Przypisanie pinów
#define DC_PIN DC_Pin
#define DC_PORT DC_GPIO_Port
#define RST_PIN RST_Pin
#define RST_PORT RST_GPIO_Port
#define CS_PIN CS_Pin
#define CS_PORT CS_GPIO_Port
#define BUSY_PIN BUSY_Pin
#define BUSY_PORT BUSY_GPIO_Port

extern const unsigned char IMAGE_BLACK[];
extern const unsigned char IMAGE_RED[];
extern SPI_HandleTypeDef hspi1;

void DigitalWrite(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState value)
{
    HAL_GPIO_WritePin(port, pin, value);
}

GPIO_PinState DigitalRead(GPIO_TypeDef *port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin);
}

void SpiTransfer(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

void DelayMs(uint32_t delay)
{
    HAL_Delay(delay);
}

void EpdIf_IfInit(void)
{
    // GPIO initialization for EPD
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, DC_Pin | RST_Pin | CS_Pin, GPIO_PIN_RESET);

    /* Configure GPIO pins : DC_Pin RST_Pin CS_Pin */
    GPIO_InitStruct.Pin = DC_Pin | RST_Pin | CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure GPIO pin : BUSY_Pin */
    GPIO_InitStruct.Pin = BUSY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUSY_GPIO_Port, &GPIO_InitStruct);
}

int Epd_Init(Epd *epd)
{

    epd->width = EPD_WIDTH;
    epd->height = EPD_HEIGHT;
    epd->reset_port = RST_PORT;
    epd->reset_pin = RST_PIN;
    epd->dc_port = DC_PORT;
    epd->dc_pin = DC_PIN;
    epd->cs_port = CS_PORT;
    epd->cs_pin = CS_PIN;
    epd->busy_port = BUSY_PORT;
    epd->busy_pin = BUSY_PIN;

    EpdIf_IfInit();

    Epd_Reset(epd);
    Epd_ReadBusy(epd);

    Epd_SendCommand(epd, 0x12);
    Epd_ReadBusy(epd);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x40);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x05);

    Epd_SendCommand(epd, 0x11);
    Epd_SendData(epd, 0x03);

    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x31);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x2B);
    Epd_SendData(epd, 0x01);

    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);
    Epd_ReadBusy(epd);

    return 0;
}

int Epd_Init_new(Epd *epd)
{
    Epd_Reset(epd);
    Epd_ReadBusy(epd);
    Epd_SendCommand(epd, 0x12);
    Epd_ReadBusy(epd);
    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x05);
    Epd_SendCommand(epd, 0x18);
    Epd_SendData(epd, 0x80);
    Epd_SendCommand(epd, 0x11);
    Epd_SendData(epd, 0x03);
    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, epd->width / 8 - 1);
    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, (epd->height - 1) % 256);
    Epd_SendData(epd, (epd->height - 1) / 256);
    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, 0x00);
    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);
    Epd_ReadBusy(epd);
    return 0;
}

int Epd_Init_old(Epd *epd)
{
    Epd_Reset(epd);
    Epd_SendCommand(epd, 0x04);
    Epd_ReadBusy(epd);
    Epd_SendCommand(epd, 0x00);
    Epd_SendData(epd, 0x0F);
    return 0;
}

void Epd_SendCommand(Epd *epd, unsigned char command)
{
    DigitalWrite(epd->dc_port, epd->dc_pin, GPIO_PIN_RESET);
    SpiTransfer(command);
}



void Epd_SendData(Epd *epd, unsigned char data)
{
    DigitalWrite(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);
    SpiTransfer(data);
}


void Epd_SendData_DMA(Epd *epd, unsigned char *data, uint16_t size)
{
    DigitalWrite(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);

    // Rozpocznij transmisję DMA
    HAL_SPI_Transmit_DMA(&hspi1, data, size);

    // Czekaj na zakończenie transmisji
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
    {
    }
}



void Epd_ReadBusy(Epd *epd)
{
    while (DigitalRead(epd->busy_port, epd->busy_pin) == GPIO_PIN_SET)
    {
        DelayMs(100);
    }
}

void Epd_Reset(Epd *epd)
{
    DigitalWrite(epd->reset_port, epd->reset_pin, GPIO_PIN_SET);
    DelayMs(200);
    DigitalWrite(epd->reset_port, epd->reset_pin, GPIO_PIN_RESET);
    DelayMs(2);
    DigitalWrite(epd->reset_port, epd->reset_pin, GPIO_PIN_SET);
    DelayMs(200);
}

void Epd_TurnOnDisplay(Epd *epd)
{
    Epd_SendCommand(epd, 0x22);
    Epd_SendData(epd, 0xF7);
    Epd_SendCommand(epd, 0x20);
    Epd_ReadBusy(epd);
}

void Epd_DisplayFrame(Epd *epd)
{
    if (epd->flag == 0)
    {
        Epd_SendCommand(epd, 0x22);
        Epd_SendData(epd, 0xF7);
        Epd_SendCommand(epd, 0x20);
        Epd_ReadBusy(epd);
    }
    else
    {
        Epd_SendCommand(epd, 0x12);
        DelayMs(100);
        Epd_ReadBusy(epd);
    }
}

void Epd_Clear(Epd *epd)
{ // funnkcja przyjmujaca wskaznik na strukture
    if (epd->flag == 0)
    {
        Epd_SendCommand(epd, 0x24);
        for (UWORD j = 0; j < epd->height; j++)
        {
            for (UWORD i = 0; i < epd->width / 8; i++)
            {
                Epd_SendData(epd, 0xFF);
            }
        }
        Epd_SendCommand(epd, 0x26);
        for (UWORD j = 0; j < epd->height; j++)
        {
            for (UWORD i = 0; i < epd->width / 8; i++)
            {
                Epd_SendData(epd, 0x00);
            }
        }
        Epd_SendCommand(epd, 0x22);
        Epd_SendData(epd, 0xF7);
        Epd_SendCommand(epd, 0x20);
        Epd_ReadBusy(epd);
    }
    else
    {
        Epd_SendCommand(epd, 0x10);
        for (UWORD j = 0; j < epd->height; j++)
        {
            for (UWORD i = 0; i < epd->width / 8; i++)
            {
                Epd_SendData(epd, 0xFF);
            }
        }
        Epd_SendCommand(epd, 0x13);
        for (UWORD j = 0; j < epd->height; j++)
        {
            for (UWORD i = 0; i < epd->width / 8; i++)
            {
                Epd_SendData(epd, 0xFF);
            }
        }
        Epd_SendCommand(epd, 0x12);
        DelayMs(100);
        Epd_ReadBusy(epd);
    }
}

void Epd_Sleep(Epd *epd)
{
    if (epd->flag == 0)
    {
        Epd_SendCommand(epd, 0x10);
        Epd_SendData(epd, 0x01);
    }
    else
    {
        Epd_SendCommand(epd, 0x50);
        Epd_SendData(epd, 0xF7);
        Epd_SendCommand(epd, 0x02);
        Epd_ReadBusy(epd);
        Epd_SendCommand(epd, 0x07);
        Epd_SendData(epd, 0xA5);
    }
}

void Epd_Display(Epd *epd, const UBYTE *image)
{
    UWORD Width, Height;
    Width = (epd->width % 8 == 0) ? (epd->width / 8) : (epd->width / 8 + 1);
    Height = epd->height;

    Epd_SendCommand(epd, 0x24);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            Epd_SendData(epd, image[i + j * Width]);
        }
    }
    Epd_SendCommand(epd, 0x26);
    for (UWORD j = 0; j < Height; j++)
    {
        for (UWORD i = 0; i < Width; i++)
        {
            Epd_SendData(epd, image[i + j * Width]);
        }
    }
    Epd_TurnOnDisplay(epd);
}

void Epd_Display_Partial_Fast(Epd *epd, const unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end)
{
    unsigned int i, width;
    unsigned int image_counter;

    if ((x_start % 8 + x_end % 8 == 8 && x_start % 8 > x_end % 8) || x_start % 8 + x_end % 8 == 0 || (x_end - x_start) % 8 == 0)
    {
        x_start = x_start / 8;
        x_end = x_end / 8;
    }
    else
    {
        x_start = x_start / 8;
        x_end = x_end % 8 == 0 ? x_end / 8 : x_end / 8 + 1;
    }

    width = x_end - x_start;
    image_counter = width * (y_end - y_start);

    x_end -= 1;
    y_end -= 1;

    Epd_Reset(epd);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, x_start & 0xff);
    Epd_SendData(epd, x_end & 0xff);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);
    Epd_SendData(epd, y_end & 0xff);
    Epd_SendData(epd, (y_end >> 8) & 0x01);

    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, x_start & 0xff);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);

    Epd_SendCommand(epd, 0x24);
    for (i = 0; i < image_counter; i++)
    {
        Epd_SendData(epd, image[i]);
    }
    Epd_TurnOnDisplay_Partial_Fast(epd);
}

void Epd_TurnOnDisplay_Partial_Fast(Epd *epd)
{
    Epd_SendCommand(epd, 0x22);
    Epd_SendData(epd, 0xC7);
    Epd_SendCommand(epd, 0x20);
    Epd_ReadBusy(epd);
}

void Epd_Display_Partial(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end)
{
    unsigned int i, width;
    unsigned int image_counter;

    if ((x_start % 8 + x_end % 8 == 8 && x_start % 8 > x_end % 8) || x_start % 8 + x_end % 8 == 0 || (x_end - x_start) % 8 == 0)
    {
        x_start = x_start / 8;
        x_end = x_end / 8;
    }
    else
    {
        x_start = x_start / 8;
        x_end = x_end % 8 == 0 ? x_end / 8 : x_end / 8 + 1;
    }

    width = x_end - x_start;
    image_counter = width * (y_end - y_start);

    x_end -= 1;
    y_end -= 1;

    Epd_Reset(epd);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, x_start & 0xff);
    Epd_SendData(epd, x_end & 0xff);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);
    Epd_SendData(epd, y_end & 0xff);
    Epd_SendData(epd, (y_end >> 8) & 0x01);

    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, x_start & 0xff);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);

    Epd_SendCommand(epd, 0x24);
    for (i = 0; i < image_counter; i++)
    {
        Epd_SendData(epd, image[i]);
    }

    
    Epd_TurnOnDisplay_Partial(epd);
}

void Epd_TurnOnDisplay_Partial(Epd *epd)
{
    Epd_SendCommand(epd, 0x22);
    Epd_SendData(epd, 0xFF);
    Epd_SendCommand(epd, 0x20);
    Epd_ReadBusy(epd);
}

void Epd_Display_Partial_Double(Epd *epd, unsigned char *image1,
                                unsigned int x1_start, unsigned int y1_start, unsigned int x1_end, unsigned int y1_end, unsigned char *image2, unsigned int x2_start, unsigned int y2_start, unsigned int x2_end, unsigned int y2_end)
{
    unsigned int i, width1, width2;
    unsigned int image1_counter, image2_counter;

    // Obliczenia dla pierwszego obszaru
    if ((x1_start % 8 + x1_end % 8 == 8 && x1_start % 8 > x1_end % 8) || x1_start % 8 + x1_end % 8 == 0 || (x1_end - x1_start) % 8 == 0)
    {
        x1_start = x1_start / 8;
        x1_end = x1_end / 8;
    }
    else
    {
        x1_start = x1_start / 8;
        x1_end = x1_end % 8 == 0 ? x1_end / 8 : x1_end / 8 + 1;
    }

    width1 = x1_end - x1_start;
    image1_counter = width1 * (y1_end - y1_start);

    x1_end -= 1;
    y1_end -= 1;

    // Obliczenia dla drugiego obszaru
    if ((x2_start % 8 + x2_end % 8 == 8 && x2_start % 8 > x2_end % 8) || x2_start % 8 + x2_end % 8 == 0 || (x2_end - x2_start) % 8 == 0)
    {
        x2_start = x2_start / 8;
        x2_end = x2_end / 8;
    }
    else
    {
        x2_start = x2_start / 8;
        x2_end = x2_end % 8 == 0 ? x2_end / 8 : x2_end / 8 + 1;
    }

    width2 = x2_end - x2_start;
    image2_counter = width2 * (y2_end - y2_start);

    x2_end -= 1;
    y2_end -= 1;

    // Reset wyświetlacza
    Epd_Reset(epd);

    // Konfiguracja wyświetlacza dla pierwszego obszaru
    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, x1_start & 0xff);
    Epd_SendData(epd, x1_end & 0xff);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, y1_start & 0xff);
    Epd_SendData(epd, (y1_start >> 8) & 0x01);
    Epd_SendData(epd, y1_end & 0xff);
    Epd_SendData(epd, (y1_end >> 8) & 0x01);

    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, x1_start & 0xff);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, y1_start & 0xff);
    Epd_SendData(epd, (y1_start >> 8) & 0x01);

    Epd_SendCommand(epd, 0x24);
    for (i = 0; i < image1_counter; i++)
    {
        Epd_SendData(epd, image1[i]);
    }

    // Konfiguracja wyświetlacza dla drugiego obszaru
    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, x2_start & 0xff);
    Epd_SendData(epd, x2_end & 0xff);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, y2_start & 0xff);
    Epd_SendData(epd, (y2_start >> 8) & 0x01);
    Epd_SendData(epd, y2_end & 0xff);
    Epd_SendData(epd, (y2_end >> 8) & 0x01);

    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, x2_start & 0xff);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, y2_start & 0xff);
    Epd_SendData(epd, (y2_start >> 8) & 0x01);

    Epd_SendCommand(epd, 0x24);
    for (i = 0; i < image2_counter; i++)
    {
        Epd_SendData(epd, image2[i]);
    }

    Epd_TurnOnDisplay_Partial(epd);
}

void Epd_DisplayFull(Epd *epd, const unsigned char *image)
{
    unsigned int width = (epd->width % 8 == 0) ? (epd->width / 8) : (epd->width / 8 + 1);
    unsigned int height = epd->height;

    Epd_SendCommand(epd, 0x24); // Start data transmission
    for (unsigned int j = 0; j < height; j++)
    {
        for (unsigned int i = 0; i < width; i++)
        {
            Epd_SendData(epd, image[i + j * width]);
        }
    }

    Epd_SendCommand(epd, 0x26); // Data transmission for the next frame
    for (unsigned int j = 0; j < height; j++)
    {
        for (unsigned int i = 0; i < width; i++)
        {
            Epd_SendData(epd, image[i + j * width]);
        }
    }

    Epd_TurnOnDisplay(epd); // Turn on display
}

void Epd_Display_Partial_Not_Refresh(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end)
{
    unsigned int i, width;
    unsigned int image_counter;

    if ((x_start % 8 + x_end % 8 == 8 && x_start % 8 > x_end % 8) || x_start % 8 + x_end % 8 == 0 || (x_end - x_start) % 8 == 0)
    {
        x_start = x_start / 8;
        x_end = x_end / 8;
    }
    else
    {
        x_start = x_start / 8;
        x_end = x_end % 8 == 0 ? x_end / 8 : x_end / 8 + 1;
    }

    width = x_end - x_start;
    image_counter = width * (y_end - y_start);

    x_end -= 1;
    y_end -= 1;

    // Reset
    Epd_Reset(epd);

    Epd_SendCommand(epd, 0x3C); // BorderWavefrom
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x44);        // set RAM x address start/end
    Epd_SendData(epd, x_start & 0xff); // RAM x address start
    Epd_SendData(epd, x_end & 0xff);   // RAM x address end

    Epd_SendCommand(epd, 0x45);               // set RAM y address start/end
    Epd_SendData(epd, y_start & 0xff);        // RAM y address start
    Epd_SendData(epd, (y_start >> 8) & 0x01); // RAM y address start
    Epd_SendData(epd, y_end & 0xff);          // RAM y address end
    Epd_SendData(epd, (y_end >> 8) & 0x01);

    Epd_SendCommand(epd, 0x4E); // set RAM x address count to 0
    Epd_SendData(epd, x_start & 0xff);

    Epd_SendCommand(epd, 0x4F); // set RAM y address count to 0
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);

    Epd_SendCommand(epd, 0x24); // Write Black and White image to RAM
    for (i = 0; i < image_counter; i++)
    {
        Epd_SendData(epd, image[i]);
    }

    // Turn on the display (partial)
    Epd_SendCommand(epd, 0x22);
    Epd_SendData(epd, 0xFF);
    Epd_SendCommand(epd, 0x20);
    Epd_ReadBusy(epd);
}



void Epd_Display_Partial_DMA(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end)
{
    static int call_count = 0; // Licznik wywołań funkcji
    unsigned int width, height, image_counter;

    // Obliczenie szerokości i wysokości
    if ((x_start % 8 + x_end % 8 == 8 && x_start % 8 > x_end % 8) || x_start % 8 + x_end % 8 == 0 || (x_end - x_start) % 8 == 0)
    {
        x_start = x_start / 8;
        x_end = x_end / 8;
    }
    else
    {
        x_start = x_start / 8;
        x_end = x_end % 8 == 0 ? x_end / 8 : x_end / 8 + 1;
    }

    width = x_end - x_start;
    height = y_end - y_start;
    image_counter = width * height;

    x_end -= 1;
    y_end -= 1;

    // Resetowanie e-papieru tylko co drugie wywołanie funkcji
    if (call_count % 120 == 0) {
        Epd_Reset(epd); 
    }
    call_count++;

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    Epd_SendCommand(epd, 0x21);
    Epd_SendData(epd, 0x00);
    Epd_SendData(epd, 0x00);

    Epd_SendCommand(epd, 0x3C);
    Epd_SendData(epd, 0x80);

    // Ustawianie okna wyświetlania
    Epd_SendCommand(epd, 0x44);
    Epd_SendData(epd, x_start & 0xff);
    Epd_SendData(epd, x_end & 0xff);

    Epd_SendCommand(epd, 0x45);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);
    Epd_SendData(epd, y_end & 0xff);
    Epd_SendData(epd, (y_end >> 8) & 0x01);

    // Ustawianie pozycji początkowej pamięci wyświetlacza
    Epd_SendCommand(epd, 0x4E);
    Epd_SendData(epd, x_start & 0xff);

    Epd_SendCommand(epd, 0x4F);
    Epd_SendData(epd, y_start & 0xff);
    Epd_SendData(epd, (y_start >> 8) & 0x01);

    // Wysyłanie danych obrazu do pamięci wyświetlacza
    Epd_SendCommand(epd, 0x24);
    
    // Użycie DMA do przesłania danych
    Epd_SendData_DMA(epd, image, image_counter);

    // Włączenie wyświetlacza
    Epd_TurnOnDisplay_Partial(epd);
}