#ifndef EPD4IN2B_H
#define EPD4IN2B_H

#include "stm32l4xx_hal.h"
#include "main.h"

// Display resolution
#define EPD_WIDTH       400
#define EPD_HEIGHT      300

#define UWORD  unsigned int
#define UBYTE  unsigned char

typedef struct {
    unsigned int width;
    unsigned int height;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
    GPIO_TypeDef* dc_port;
    uint16_t dc_pin;
    GPIO_TypeDef* cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef* busy_port;
    uint16_t busy_pin;
    unsigned char flag;
} Epd;

int Epd_Init(Epd* epd);
void Epd_DisplayFrame(Epd* epd);
int Epd_Init_new(Epd* epd);
int Epd_Init_old(Epd* epd);
void Epd_SendCommand(Epd* epd, unsigned char command);
void Epd_SendData(Epd* epd, unsigned char data);
void Epd_ReadBusy(Epd* epd);
void Epd_Reset(Epd* epd);
void Epd_Display_Window_Black(Epd* epd, const UBYTE *image, UBYTE count);
void Epd_Display_Window_Red_new(Epd* epd, const UBYTE *image, UBYTE count);
void Epd_Display_Window_Red_old(Epd* epd, const UBYTE *image, UBYTE count);
void Epd_Display_Window_Red(Epd* epd, const UBYTE *image, UBYTE count);
void Epd_Display_old(Epd* epd, const UBYTE *blackimage, const UBYTE *ryimage);
void Epd_Display_new(Epd* epd, const UBYTE *blackimage, const UBYTE *ryimage);
void Epd_DisplayFull(Epd *epd, const unsigned char *image);
void Epd_Clear(Epd* epd);
void Epd_Sleep(Epd* epd);

void DigitalWrite(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
GPIO_PinState DigitalRead(GPIO_TypeDef* port, uint16_t pin);
void SpiTransfer(uint8_t data);
void DelayMs(uint32_t ms);
void EpdIf_IfInit(void);

void Epd_Display(Epd *epd, const unsigned char *image);
void Epd_Display_Partial(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end);
void Epd_Display_Partial_DMA(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end);

void Epd_TurnOnDisplay_Partial(Epd *epd);
void Epd_TurnOnDisplay(Epd *epd);


void Epd_Display_Partial_Not_Refresh(Epd *epd, unsigned char *image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end);

void Epd_Display_Partial_Double(Epd *epd, unsigned char *image1, unsigned int x1_start, unsigned int y1_start, unsigned int x1_end, unsigned int y1_end, unsigned char *image2, unsigned int x2_start, unsigned int y2_start, unsigned int x2_end, unsigned int y2_end);




void Epd_Display_Partial_Fast(Epd *epd, const unsigned char* image, unsigned int x_start, unsigned int y_start, unsigned int x_end, unsigned int y_end);


void Epd_TurnOnDisplay_Partial_Fast(Epd *epd);






#endif /* EPD4IN2B_H */