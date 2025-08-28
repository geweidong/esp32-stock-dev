#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
#include "GxEPD2_display_selection_new_style.h"

#define ROTATION 0

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display(GxEPD2_DRIVER_CLASS(/*CS=D8*/ 5, /*DC=D3*/ 17, /*RST=D4*/ 16, /*BUSY=D2*/ 4));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

int _screen_status = -1;
TaskHandle_t SCREEN_HANDLER;

void init_cal_layout_size() {

}

void draw_cal_layout() {
    uint16_t color;

    u8g2Fonts.setFont(u8g2_font_open_iconic_all_2x_t);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);

}

void draw_weather() {
}

// Draw err
void draw_err(bool partial) {
    if (partial) {
        display.setPartialWindow(380, 0, 20, 20);
        display.firstPage();
        display.fillScreen(GxEPD_WHITE);
    }
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setForegroundColor(GxEPD_RED);
    u8g2Fonts.setFont(u8g2_font_open_iconic_all_2x_t);
    u8g2Fonts.setCursor(382, 18);
    u8g2Fonts.print("\u0118");

    if (partial) {
        display.nextPage();
    }
}

///////////// Calendar //////////////
/**
 * 处理日历信息
 */
void si_calendar() {
}

///////////// Screen //////////////
/**
 * 屏幕刷新
 */
void task_screen(void* param) {
    Serial.println("[Task] screen update begin...");

    display.init(115200);          // 串口使能 初始化完全刷新使能 复位时间 ret上拉使能
    display.setRotation(ROTATION); // 设置屏幕旋转1和3是横向  0和2是纵向
    u8g2Fonts.begin(display);

    init_cal_layout_size();
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    draw_cal_layout();

    display.display();

    Serial.println("[Task] screen update end...");
    vTaskDelete(NULL);
}

void si_screen() {
    _screen_status = 0;
    si_calendar(); // 准备日历数据
    xTaskCreate(task_screen, "Screen", 4096, NULL, 2, &SCREEN_HANDLER);
}

int si_screen_status() {
    return _screen_status;
}