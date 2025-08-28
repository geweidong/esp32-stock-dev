// =====================
// 引入头文件
// =====================
#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
#include "GxEPD2_display_selection_new_style.h"

// =====================
// 宏定义
// =====================
#define ROTATION 0

// =====================
// 全局变量
// =====================
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display(GxEPD2_DRIVER_CLASS(/*CS=D8*/ 5, /*DC=D3*/ 17, /*RST=D4*/ 16, /*BUSY=D2*/ 4));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

int _screen_status = -1;
TaskHandle_t SCREEN_HANDLER;

// =====================
// 函数声明
// =====================
void init_cal_layout_size();
void draw_cal_layout();
void draw_weather();
void draw_err(bool partial);
void si_calendar();
void task_screen(void* param);
void si_screen();
int si_screen_status();
void draw_stock_info(const String& name, const String& price, const String& rate);

// =====================
// 函数实现
// =====================
// 初始化布局尺寸
void init_cal_layout_size() {

}

// 绘制日历布局
void draw_cal_layout() {
    uint16_t color;

    u8g2Fonts.setFont(u8g2_font_open_iconic_all_2x_t);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);

}

// 绘制天气信息
void draw_weather() {
}

// 绘制错误信息
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

// 处理日历信息
void si_calendar() {
}

// 屏幕刷新任务
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

// 屏幕刷新接口
void si_screen() {
    _screen_status = 0;
    si_calendar(); // 准备日历数据
    xTaskCreate(task_screen, "Screen", 4096, NULL, 2, &SCREEN_HANDLER);
}

// 获取屏幕状态
int si_screen_status() {
    return _screen_status;
}

// 绘制股票信息
void draw_stock_info(const String& name, const String& price, const String& rate) {
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // 设置字体和颜色
    u8g2Fonts.setFont(u8g2_font_ncenB14_tr); // 设置字体
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    // 绘制股票名称
    u8g2Fonts.setCursor(10, 30);
    u8g2Fonts.print("名称: " + name);

    // 绘制股票价格
    u8g2Fonts.setCursor(10, 60);
    u8g2Fonts.print("价格: " + price);

    // 绘制涨跌幅
    u8g2Fonts.setCursor(10, 90);
    u8g2Fonts.print("涨跌幅: " + rate + "%");

    // 刷新显示
    display.display();
}