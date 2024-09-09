#ifndef _EPDPAINT_H_
#define _EPDPAINT_H_




#include "fonts.h" // Dodanie tego nagłówka, aby użyć sFONT z tego pliku



typedef struct {
    unsigned char* image;
    int width;
    int height;
    int rotate;
} Paint;




void Paint_Init(Paint* paint, unsigned char* image, int width, int height);
void Paint_Clear(Paint* paint, int colored);
void Paint_DrawPixel(Paint* paint, int x, int y, int colored);
void Paint_DrawCharAt(Paint* paint, int x, int y, char ascii_char, sFONT* font, int colored);
void Paint_DrawStringAt(Paint* paint, int x, int y, const char* text, sFONT* font, int colored);
void Paint_DrawHorizontalLine(Paint* paint, int x, int y, int width, int colored);
void Paint_DrawVerticalLine(Paint* paint, int x, int y, int height, int colored);
void Paint_DrawRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored);
void Paint_DrawRoundedRectangle(Paint* paint, int x0, int y0, int x1, int y1, int r, int colored);
void Paint_DrawFilledRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored);
void Paint_DrawLine(Paint* paint, int x0, int y0, int x1, int y1, int colored);
void Paint_DrawImage(Paint* paint, const unsigned char* image_buffer, int x, int y, int image_width, int image_height);
void Paint_DrawCircle(Paint* paint, int x, int y, int radius, int colored);
void Paint_DrawFilledCircle(Paint* paint, int x, int y, int radius, int colored);

void Paint_DrawCircleQuarter(Paint* paint, int x, int y, int r, int quarter, int colored);

void DrawBattery(Paint *paint, int x, int y, int width, int height, float percentage, int colored);



void Paint_DrawBitmap(Paint* paint, const unsigned char* bitmap, int x, int y, int width, int height, int colored);




void DrawIcon(Paint *paint, const unsigned char *icon, int x, int y, int width, int height, int selected);




void Paint_DrawStringAtCenter(Paint* paint, int y, const char* text, const sFONT* font, int displayWidth);

void DrawTopPanel(Paint* paint, int counter, float batteryLevel, uint32_t encoderValue, int iconIndex);
void DrawBottomPanel(Paint* paint, int iconIndex);


void Paint_SetWidth(Paint* paint, int width);
void Paint_SetHeight(Paint* paint, int height);
int Paint_GetWidth(Paint* paint);
int Paint_GetHeight(Paint* paint);
void Paint_SetRotate(Paint* paint, int rotate);
int Paint_GetRotate(Paint* paint);
unsigned char* Paint_GetImage(Paint* paint);




void Paint_DrawLineWithThickness(Paint *paint, int x0, int y0, int x1, int y1, int thickness, int colored);

void Paint_DrawRectangleWithThickness(Paint *paint, int x0, int y0, int x1, int y1, int thickness, int colored);
void Paint_Draw3RectanglesCenter(Paint *paint, int y0_bottom, int height, int distance, int thickness, int colored, int x0_left, int x0_right);
void Paint_Universal_Ring(Paint *paint, int x0, int y0, int width, int height, int thickness, int colored, int type);

void Paint_DrawRing(Paint *paint, int x0, int y0, int radius, int thickness, int colored);






void Paint_DrawStringWithOutline(Paint *paint, int x, int y, const char *text, const sFONT *font, int outline_width);
    // Rysuj tekst białym kolorem (tło) w czterech kierunkach, aby stworzyć obrys







#endif /* _EPDPAINT_H_ */