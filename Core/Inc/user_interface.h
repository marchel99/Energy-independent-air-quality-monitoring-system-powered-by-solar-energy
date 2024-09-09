#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "main.h"
#include "globals.h"
#include "epdpaint.h"
#define COLORED     1
#define UNCOLORED   0




void ShowMenu1(void);
void ShowMenu2(void);
void ShowMenu3(void);
void ShowMenu4(void);
void ShowMenu5(void);
void ShowMenu6(void);
void ShowMenu7(void);
void ShowMenu8(void);




bool is_button_pressed(void);


const char* getMonthStr(uint8_t month);
void UI_HandleButtonPress_2(void); 


void AdjustValue(int);
void UI_HandleButtonPress_3(void); 
int CanExitMenu3(void);
void EditMenu3Setting();
void EditHourSetting(void);
void EditMinuteSetting(void);
void EditDaySetting(void);
void EditMonthSetting(void);
void EditYearSetting(void);




void UI_HandleButtonPress_4(void); 
void UI_HandleButtonPress_5(void); 
void UI_HandleButtonPress_6(void); 
void UI_HandleButtonPress_7(void); 
void UI_HandleButtonPress_8(void); 

void DisplayTopSection(Paint* paint_top, int iconIndex, uint32_t encoderValue, int counter, uint8_t batteryLevel);
void DisplayMiddleSection(Paint* paint_top);

void DisplayBottomSection(Paint* paint_top, int iconIndex);
void UpdateBatteryLevel(uint8_t* batteryLevel);

int getIconIndex(uint32_t encoderValue);

extern int buttonState;
extern int currentDisplayMode;
extern volatile int currentIconIndex;
extern volatile uint8_t inMenu;


#endif // USER_INTERFACE_H