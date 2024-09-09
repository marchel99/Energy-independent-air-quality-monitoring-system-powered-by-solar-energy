#include "epdpaint.h"
#include <stdlib.h>
#include <string.h>
#include "imagedata.h"
#include "epd4in2b.h"
#include "user_interface.h"


void Paint_Init(Paint* paint, unsigned char* image, int width, int height) {
    paint->image = image;
    paint->width = (width % 8) ? (width + 8 - (width % 8)) : width;
    paint->height = height;
    paint->rotate = 0;
}

void Paint_Clear(Paint* paint, int colored) {
    for (int x = 0; x < paint->width; x++) {
        for (int y = 0; y < paint->height; y++) {
            Paint_DrawPixel(paint, x, y, colored);
        }
    }
}

void Paint_DrawPixel(Paint* paint, int x, int y, int colored) {
    if (x < 0 || x >= paint->width || y < 0 || y >= paint->height) {
        return;
    }

    int new_x = x;
    int new_y = y;

    switch (paint->rotate) {
        case 90:
            new_x = paint->width - 1 - y;
            new_y = x;
            break;
        case 180:
            new_x = paint->width - 1 - x;
            new_y = paint->height - 1 - y;
            break;
        case 270:
            new_x = y;
            new_y = paint->height - 1 - x;
            break;
    }

    if (colored) {
        paint->image[(new_x + new_y * paint->width) / 8] &= ~(0x80 >> (new_x % 8));
    } else {
        paint->image[(new_x + new_y * paint->width) / 8] |= 0x80 >> (new_x % 8);
    }
}

void Paint_DrawCharAt(Paint* paint, int x, int y, char ascii_char, sFONT* font, int colored) {
    int i, j;
    unsigned int offset = (ascii_char - ' ') * font->Height * ((font->Width + 7) / 8);
    const unsigned char* ptr = &font->table[offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if (*ptr & (0x80 >> (i % 8))) {
                Paint_DrawPixel(paint, x + i, y + j, colored);
            }
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}
void Paint_DrawStringAt(Paint* paint, int x, int y, const char* text, sFONT* font, int colored) {
    const char* p_text = text;
    unsigned int refcolumn = x;

    while (*p_text != 0) {
        Paint_DrawCharAt(paint, refcolumn, y, *p_text, font, colored);
        refcolumn += font->Width;
        p_text++;
    }
}


void Paint_DrawHorizontalLine(Paint* paint, int x, int y, int width, int colored) {
    for (int i = x; i < x + width; i++) {
        Paint_DrawPixel(paint, i, y, colored);
    }
}

void Paint_DrawVerticalLine(Paint* paint, int x, int y, int height, int colored) {
    for (int i = y; i < y + height; i++) {
        Paint_DrawPixel(paint, x, i, colored);
    }
}

void Paint_DrawRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    int min_x = (x0 < x1) ? x0 : x1;
    int max_x = (x0 > x1) ? x0 : x1;
    int min_y = (y0 < y1) ? y0 : y1;
    int max_y = (y0 > y1) ? y0 : y1;

    Paint_DrawHorizontalLine(paint, min_x, min_y, max_x - min_x + 1, colored);
    Paint_DrawHorizontalLine(paint, min_x, max_y, max_x - min_x + 1, colored);
    Paint_DrawVerticalLine(paint, min_x, min_y, max_y - min_y + 1, colored);
    Paint_DrawVerticalLine(paint, max_x, min_y, max_y - min_y + 1, colored);
}

// rectangle with padding

void Paint_DrawRoundedRectangle(Paint* paint, int x0, int y0, int x1, int y1, int r, int colored) {
    // Rysowanie prostych odcinków
    Paint_DrawHorizontalLine(paint, x0 + r, y0, x1 - x0 - 2 * r + 1, colored);
    Paint_DrawHorizontalLine(paint, x0 + r, y1, x1 - x0 - 2 * r + 1, colored);
    Paint_DrawVerticalLine(paint, x0, y0 + r, y1 - y0 - 2 * r + 1, colored);
    Paint_DrawVerticalLine(paint, x1, y0 + r, y1 - y0 - 2 * r + 1, colored);

    // Rysowanie zaokrąglonych rogów
    Paint_DrawCircleQuarter(paint, x0 + r, y0 + r, r, 1, colored); // Lewy górny róg
    Paint_DrawCircleQuarter(paint, x1 - r, y0 + r, r, 2, colored); // Prawy górny róg
    Paint_DrawCircleQuarter(paint, x0 + r, y1 - r, r, 3, colored); // Lewy dolny róg
    Paint_DrawCircleQuarter(paint, x1 - r, y1 - r, r, 4, colored); // Prawy dolny róg
}

// Funkcja rysująca ćwiartkę okręgu
void Paint_DrawCircleQuarter(Paint* paint, int x, int y, int r, int quarter, int colored) {
    int x_pos = r, y_pos = 0;
    int err = 0;

    while (x_pos >= y_pos) {
        if (quarter == 1) {
            Paint_DrawPixel(paint, x - x_pos, y - y_pos, colored);
            Paint_DrawPixel(paint, x - y_pos, y - x_pos, colored);
        } else if (quarter == 2) {
            Paint_DrawPixel(paint, x + x_pos, y - y_pos, colored);
            Paint_DrawPixel(paint, x + y_pos, y - x_pos, colored);
        } else if (quarter == 3) {
            Paint_DrawPixel(paint, x - x_pos, y + y_pos, colored);
            Paint_DrawPixel(paint, x - y_pos, y + x_pos, colored);
        } else if (quarter == 4) {
            Paint_DrawPixel(paint, x + x_pos, y + y_pos, colored);
            Paint_DrawPixel(paint, x + y_pos, y + x_pos, colored);
        }

        if (err <= 0) {
            y_pos += 1;
            err += 2 * y_pos + 1;
        }

        if (err > 0) {
            x_pos -= 1;
            err -= 2 * x_pos + 1;
        }
    }
}



void Paint_DrawFilledRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    int min_x = (x0 < x1) ? x0 : x1;
    int max_x = (x0 > x1) ? x0 : x1;
    int min_y = (y0 < y1) ? y0 : y1;
    int max_y = (y0 > y1) ? y0 : y1;

    for (int i = min_x; i <= max_x; i++) {
        Paint_DrawVerticalLine(paint, i, min_y, max_y - min_y + 1, colored);
    }
}

void Paint_DrawLine(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        Paint_DrawPixel(paint, x0, y0, colored);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Paint_DrawImage(Paint* paint, const unsigned char* image_buffer, int x, int y, int image_width, int image_height) {
    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            if (image_buffer[i + j * image_width] == 0) {
                Paint_DrawPixel(paint, x + i, y + j, COLORED);
            } else {
                Paint_DrawPixel(paint, x + i, y + j, UNCOLORED);
            }
        }
    }
}

void Paint_DrawCircle(Paint* paint, int x, int y, int radius, int colored) {
    int xi = -radius;
    int yi = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        Paint_DrawPixel(paint, x - xi, y + yi, colored);
        Paint_DrawPixel(paint, x + xi, y + yi, colored);
        Paint_DrawPixel(paint, x + xi, y - yi, colored);
        Paint_DrawPixel(paint, x - xi, y - yi, colored);
        e2 = err;
        if (e2 <= yi) {
            err += ++yi * 2 + 1;
            if (-xi == yi && e2 <= xi) {
                e2 = 0;
            }
        }
        if (e2 > xi) {
            err += ++xi * 2 + 1;
        }
    } while (xi <= 0);
}

void Paint_DrawFilledCircle(Paint* paint, int x, int y, int radius, int colored) {
    int xi = -radius;
    int yi = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        Paint_DrawPixel(paint, x - xi, y + yi, colored);
        Paint_DrawPixel(paint, x + xi, y + yi, colored);
        Paint_DrawPixel(paint, x + xi, y - yi, colored);
        Paint_DrawPixel(paint, x - xi, y - yi, colored);
        Paint_DrawLine(paint, x - xi, y + yi, x + xi, y + yi, colored);
        Paint_DrawLine(paint, x - xi, y - yi, x + xi, y - yi, colored);
        e2 = err;
        if (e2 <= yi) {
            err += ++yi * 2 + 1;
            if (-xi == yi && e2 <= xi) {
                e2 = 0;
            }
        }
        if (e2 > xi) {
            err += ++xi * 2 + 1;
        }
    } while (xi <= 0);
}

void Paint_SetWidth(Paint* paint, int width) {
    paint->width = (width % 8) ? (width + 8 - (width % 8)) : width;
}

void Paint_SetHeight(Paint* paint, int height) {
    paint->height = height;
}

int Paint_GetWidth(Paint* paint) {
    return paint->width;
}

int Paint_GetHeight(Paint* paint) {
    return paint->height;
}

void Paint_SetRotate(Paint* paint, int rotate) {
    paint->rotate = rotate;
}

int Paint_GetRotate(Paint* paint) {
    return paint->rotate;
}

unsigned char* Paint_GetImage(Paint* paint) {
    return paint->image;
}
void DrawBattery(Paint *paint, int x, int y, int width, int height, float percentage, int colored) {
    // Główny prostokąt baterii
    Paint_DrawRectangle(paint, x, y, x + width, y + height, colored);

    // Mały prostokąt jako złącze baterii
    int connectorWidth = 5;
    int connectorHeight = height / 2;
    Paint_DrawRectangle(paint, x + width, y + (height - connectorHeight) / 2, x + width + connectorWidth, y + (height + connectorHeight) / 2, colored);

    // Definiowanie offsetu
    int offset = 2;

    // Obliczanie szerokości wypełnienia odpowiadającej poziomowi naładowania
    int fillWidth = (width - 2 * offset) * percentage / 100;

    // Rysowanie wypełnienia baterii z offsetem
    Paint_DrawFilledRectangle(paint, x + offset, y + offset, x + offset + fillWidth, y + height - offset, colored); // Wypełnienie prostokąta wewnątrz baterii
}



void DrawBatteryLevel(Paint *paint, int x, int y, int width, int height, int level, int colored) {
    // Szerokość jednego poziomu naładowania
    int levelWidth = (width - 4) / 3; // -4 to marginesy
    int gap = 2; // Odstęp między poziomami naładowania

    for (int i = 0; i < level; i++) {
        // Rysuj poziomy naładowania
        int levelX = x + 2 + i * (levelWidth + gap); // Pozycja X z uwzględnieniem odstępów
        Paint_DrawFilledRectangle(paint, levelX, y + 2, levelX + levelWidth, y + height - 2, colored); // -2 to górny i dolny margines
    }
}



void Paint_DrawBitmap(Paint* paint, const unsigned char* bitmap, int x, int y, int width, int height, int colored) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (bitmap[(i / 8) + j * (width / 8)] & (1 << (7 - (i % 8)))) {
                Paint_DrawPixel(paint, x + i, y + j, !colored);
            } else {
                Paint_DrawPixel(paint, x + i, y + j, colored);
            }
        }
    }
}








void Paint_DrawStringAtCenter(Paint* paint, int y, const char* text, const sFONT* font, int displayWidth) {
    int textLength = strlen(text);
    int textWidth = textLength * font->Width;
    int centeredX = (displayWidth - textWidth) / 2+9;
    Paint_DrawStringAt(paint, centeredX, y, text, font, COLORED);
}













void DrawIcon(Paint *paint, const unsigned char *icon, int x, int y, int width, int height, int selected) {
    if (selected) {
        // Rysuj migający prostokąt wokół ikony
        Paint_DrawRectangle(paint, x - 2, y - 2, x + width + 2, y + height + 2, COLORED);
    }
    Paint_DrawBitmap(paint, icon, x, y, width, height, COLORED);
}






void DrawBottomPanel(Paint* paint, int iconIndex) {
    const char* menuText;

    // Wybór napisu w zależności od wartości iconIndex
    switch (iconIndex) {
        case 1:
            menuText = "Menu 1";
            break;
        case 2:
            menuText = "Menu 2";
            break;
        case 3:
            menuText = "Menu 3";
            break;
        case 4:
            menuText = "Menu 4";
            break;
        case 5:
            menuText = "Menu 5";
            break;
        case 6:
            menuText = "Menu 6";
            break;
        case 7:
            menuText = "Menu 7";
            break;
        case 8:
            menuText = "Menu 8";
            break;
        default:
            menuText = "Unknown Menu";
            break;
    }

    // Obliczanie pozycji X, aby tekst był wyśrodkowany
    int textLength = strlen(menuText);
    int textWidth = textLength * Font16.Width; // Zakładając, że używasz Font16
    int centeredX = (400 - textWidth) / 2;

    // Wyświetlanie napisu na środku ekranu
    Paint_DrawStringAt(paint, centeredX, 275, menuText, &Font16, COLORED);
}



void DrawTopPanel(Paint* paint, int counter, float batteryLevel, uint32_t encoderValue, int iconIndex) {
    char buffer_top[100];
    
    // Wyświetlanie informacji o ikonie i wartości enkodera
    snprintf(buffer_top, sizeof(buffer_top), "I%d, E:%lu", iconIndex, encoderValue);
    Paint_DrawStringAt(paint, 150, 5, buffer_top, &Font20, COLORED);

    // Wyświetlanie licznika
    snprintf(buffer_top, sizeof(buffer_top), "%d", counter);
    Paint_DrawStringAt(paint, 10, 5, buffer_top, &Font20, COLORED);

    // Rysowanie poziomu baterii
    DrawBattery(paint, 350, 2, 32, 70,batteryLevel, COLORED);
   
}










void Paint_DrawLineWithThickness(Paint *paint, int x0, int y0, int x1, int y1, int thickness, int colored)
{
    if (x0 == x1)
    { // Linia pionowa
        for (int i = -thickness / 2; i <= thickness / 2; i++)
        {
            Paint_DrawLine(paint, x0 + i, y0, x1 + i, y1, colored);
        }
    }
    else if (y0 == y1)
    { // Linia pozioma
        for (int i = -thickness / 2; i <= thickness / 2; i++)
        {
            Paint_DrawLine(paint, x0, y0 + i, x1, y1 + i, colored);
        }
    }
    else
    { // Linia ukośna
        for (int i = -thickness / 2; i <= thickness / 2; i++)
        {
            Paint_DrawLine(paint, x0, y0 + i, x1, y1 + i, colored);
            Paint_DrawLine(paint, x0 + i, y0, x1 + i, y1, colored);
        }
    }
}



void Paint_DrawRectangleWithThickness(Paint *paint, int x0, int y0, int x1, int y1, int thickness, int colored)
{
    Paint_DrawLineWithThickness(paint, x0, y0, x1, y0, thickness, colored); // Top
    Paint_DrawLineWithThickness(paint, x0, y0, x0, y1, thickness, colored); // Left
    Paint_DrawLineWithThickness(paint, x1, y0, x1, y1, thickness, colored); // Right
    Paint_DrawLineWithThickness(paint, x0, y1, x1, y1, thickness, colored); // Bottom
}





void Paint_Draw3RectanglesCenter(Paint *paint, int y0_bottom, int height, int distance, int thickness, int colored, int x0_left, int x0_right)
{
    int width = x0_right - x0_left; // Całkowita szerokość dla prostokątów i odstępów
    int rect_width = (width - 2 * distance) / 3; // Szerokość pojedynczego prostokąta
    int start_x = x0_left; // Punkt początkowy dla lewego prostokąta

    for (int i = 0; i < 3; i++)
    {
        int x0 = start_x + i * (rect_width + distance);
        int x1 = x0 + rect_width;
        int y0 = y0_bottom; // Pozycja y górnej krawędzi prostokąta
        int y1 = y0 + height; // Pozycja y dolnej krawędzi prostokąta
        
        // Sprawdzenie, czy prostokąt nie wyjdzie poza ograniczenie x0_right
        if (x1 > x0_right) {
            x1 = x0_right;
        }

        Paint_DrawRectangleWithThickness(paint, x0, y0, x1, y1, thickness, colored);
    }
}




void Paint_DrawRing(Paint *paint, int x0, int y0, int radius, int thickness, int colored)
{
    int inner_radius = radius - thickness;
    int outer_radius = radius;
    int x, y;

    for (y = -outer_radius; y <= outer_radius; y++)
    {
        for (x = -outer_radius; x <= outer_radius; x++)
        {
            int distance = x * x + y * y;
            if (distance <= outer_radius * outer_radius && distance >= inner_radius * inner_radius)
            {
                Paint_DrawPixel(paint, x0 + x, y0 + y, colored);
            }
        }
    }
}





void Paint_Universal_Ring(Paint *paint, int x0, int y0, int width, int height, int thickness, int colored, int type)
{
    int radius = height; // Promień okręgu to wysokość prostokąta
    int x1 = x0 + width;
    int y1 = y0 + height;
    int cx, cy;

    if (type == 1 || type == 3)
    {
        // Lewa strona
        cx = x0 + radius;
    }
    else
    {
        // Prawa strona
        cx = x1 - radius;
    }

    if (type == 1 || type == 2)
    {
        // Góra
        cy = y0 + radius;
    }
    else
    {
        // Dół
        cy = y1 - radius;
    }

    // Rysowanie ćwiartki pierścienia
    int inner_radius = radius - thickness;
    int outer_radius = radius;
    for (int y = -outer_radius; y <= 0; y++)
    {
        for (int x = -outer_radius; x <= 0; x++)
        {
            int distance = x * x + y * y;

            if (distance <= outer_radius * outer_radius && distance >= inner_radius * inner_radius)
            {
                if (type == 1 || type == 3)
                {
                    // Lewa górna lub lewa dolna (ćwiartka przesunięta w prawo)
                    Paint_DrawPixel(paint, cx + x + radius, cy + (type == 1 ? y : -y), colored);
                }
                else
                {
                    // Prawa górna lub prawa dolna
                    Paint_DrawPixel(paint, cx + (type == 1 || type == 3 ? x : -x - radius), cy + (type == 1 || type == 2 ? y : -y), colored);
                }
            }
        }
    }

    // Rysowanie pionowych linii prostokąta
    if (type == 1 || type == 3)
    {
        Paint_DrawLineWithThickness(paint, x0, y0, x0, y1, thickness, colored); // Lewa pionowa linia
    }
    else
    {
        Paint_DrawLineWithThickness(paint, x1, y0, x1, y1, thickness, colored); // Prawa pionowa linia
    }

    // Rysowanie górnej lub dolnej linii prostokąta (krótkiej i długiej)
    for (int i = 0; i < thickness; i++)
    {
        if (type == 1 || type == 2)
        {
            // Długa górna linia
            Paint_DrawLineWithThickness(paint, type == 2 ? cx - radius : x0, y0 + i, type == 1 ? cx + radius : x1, y0 + i, 1, colored);

            // Krótka dolna linia
            Paint_DrawLineWithThickness(paint, type == 1 ? x0 : x1 - radius, y1 + i - thickness + 1, type == 1 ? cx : x1, y1 + i - thickness + 1, 1, colored);
            
        }
        else
        {
            // Krótka górna linia
            Paint_DrawLineWithThickness(paint, type == 3 ? x0 : x1 - radius, y0 + i, type == 3 ? cx : x1, y0 + i, 1, colored);

            // Długa dolna linia
            Paint_DrawLineWithThickness(paint, type == 4 ? cx - radius : x0, y1 + i - thickness + 1, type == 3 ? cx + radius : x1, y1 + i - thickness + 1, 1, colored);
        }
    }
}

void Paint_DrawStringWithOutline(Paint *paint, int x, int y, const char *text, const sFONT *font, int outline_width) {
    // Rysuj tekst białym kolorem (tło) w ośmiu kierunkach, aby stworzyć bardziej jednolity obrys
    Paint_DrawStringAt(paint, x - outline_width, y, text, font, COLORED); // W lewo
    Paint_DrawStringAt(paint, x + outline_width, y, text, font, COLORED); // W prawo
    Paint_DrawStringAt(paint, x, y - outline_width, text, font, COLORED); // W górę
    Paint_DrawStringAt(paint, x, y + outline_width, text, font, COLORED); // W dół

    Paint_DrawStringAt(paint, x - outline_width, y - outline_width, text, font, COLORED); // W lewo-góra
    Paint_DrawStringAt(paint, x + outline_width, y - outline_width, text, font, COLORED); // W prawo-góra
    Paint_DrawStringAt(paint, x - outline_width, y + outline_width, text, font, COLORED); // W lewo-dół
    Paint_DrawStringAt(paint, x + outline_width, y + outline_width, text, font, COLORED); // W prawo-dół

    // Rysuj tekst w środku czarnym kolorem (tekst właściwy)
    Paint_DrawStringAt(paint, x, y, text, font, UNCOLORED);
}
