// =====================
// 引入头文件
// =====================
#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
#include "GxEPD2_display_selection_new_style.h"
#include "stock_data.h"

// =====================
// 宏定义
// =====================
#include "font.h"
#define ROTATION 0
#define FONT_TEXT u8g2_font_wqy16_t_gb2312 // 224825bytes，最大字库（天气描述中“霾”，只有此字库中有）
// =====================
// 全局变量
// =====================
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display(GxEPD2_DRIVER_CLASS(
    /*CS=D8*/ 5,
    /*DC=D3*/ 17,
    /*RST=D4*/ 16,
    /*BUSY=D2*/ 4
));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

int _screen_status = -1;
TaskHandle_t SCREEN_HANDLER;

// =====================
// 函数声明
// =====================
void draw_err(bool partial);
int si_screen_status();
void draw_stock_data(const StockData& stockData);
void task_stock_screen(void* param);
void si_stock_screen(const StockData& stockData);
// =====================
// 函数实现
// =====================
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
// 获取屏幕状态
int si_screen_status() {
    return _screen_status;
}

// 绘制完整股票数据
void draw_stock_data(const StockData& stockData) {
    if (!stockData.isValid) {
        Serial.println("[Screen] 股票数据无效，显示错误信息");
        draw_err(false);
        return;
    }

    Serial.println("[Screen] 开始绘制股票数据: " + stockData.name);
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);  // 白色背景

    // 设置基础字体样式
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 白色背景

    // 绘制文字
    u8g2Fonts.setFont(u8g2_font_ncenB18_tr);  // 使用支持ASCII的字体
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // 黑色前景色
    u8g2Fonts.setCursor(10, 50);
    u8g2Fonts.print("STOCK DATA");  // 先用英文测试
    
    // 添加股票信息
    u8g2Fonts.setFont(u8g2_font_ncenB14_tr);
    u8g2Fonts.setCursor(10, 80);
    u8g2Fonts.print("Name: " + stockData.name);

    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.print(stockData.name);
    
    u8g2Fonts.setCursor(10, 110);
    u8g2Fonts.print("Price: " + stockData.price);
    
    u8g2Fonts.setCursor(10, 140);
    u8g2Fonts.print("Change: " + stockData.rate + "%");

    u8g2Fonts.setCursor(10, 170);
    u8g2Fonts.print("Volume: " + stockData.volume);

    u8g2Fonts.setCursor(10, 200);
    u8g2Fonts.print("Time: " + stockData.time);

    // 刷新显示
    display.display();
    Serial.println("[Screen] 股票数据显示完成");
}

// 股票屏幕刷新任务
void task_stock_screen(void* param) {
    StockData* stockData = (StockData*)param;
    
    Serial.println("[Task] 股票屏幕更新开始...");

    display.init(115200);          // 初始化显示
    display.setRotation(ROTATION); // 设置屏幕旋转
    u8g2Fonts.begin(display);

    draw_stock_data(*stockData);

    _screen_status = 1; // 设置为完成状态
    Serial.println("[Task] 股票屏幕更新结束");
    
    delete stockData; // 释放内存
    vTaskDelete(NULL);
}

// 股票屏幕刷新接口
void si_stock_screen(const StockData& stockData) {
    _screen_status = 0; // 设置为进行中状态
    
    // 复制股票数据到堆内存，避免栈变量被释放
    StockData* stockDataCopy = new StockData(stockData);
    
    xTaskCreate(task_stock_screen, "StockScreen", 8192, stockDataCopy, 2, &SCREEN_HANDLER);
}
