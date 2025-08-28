#include <Arduino.h>
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "GxEPD2_display_selection_new_style.h"
#include <WiFi.h>
#include <WiFiManager.h> // 用于WiFi连接管理
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "_preference.h"
#include "stock_data.h"

// =====================
// 引脚定义
// =====================
#define PIN_CS   5    // 片选
#define PIN_DC   17   // 数据/命令
#define PIN_RST  16   // 复位
#define PIN_BUSY 4    // BUSY信号

// =====================
// 全局变量
// =====================
WiFiManager wm;
WiFiManagerParameter para_stock_api_key("stock_api_key", "api key", "", 64); // API Key
WiFiManagerParameter para_stock_code("stock_code", "股票code", "", 32); // 股票代码

String apiUrl = "http://web.juhe.cn/finance/stock/hs?key=87e463bjj90112bd44da4bc4154ad8544dda&gid=sh601009&type=";
unsigned long previousMillis = 0;
const long interval = 60000; // 更新间隔（毫秒）
bool _wifi_flag = false; // WiFi连接状态

// =====================
// 外部函数声明
// =====================
extern void si_stock_screen(const StockData& stockData);
extern int si_screen_status();

// =====================
// 函数声明
// =====================
void fetchAndDisplayStockData();
void displayMockStockData();
void preSaveParamsCallback();
void saveParamsCallback();
void setupWiFiManager();

// =====================
// 函数实现
// =====================
// 获取并显示股票数据
void fetchAndDisplayStockData() {
  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  String stockApiKey = pref.getString(PREF_STOCK_API_KEY);
  String stockCode = pref.getString(PREF_STOCK_CODE);
  pref.end();

  StockData stockData;
  stockData.isValid = false;

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("尝试获取真实股票数据...");
    HTTPClient http;
    String requestUrl = apiUrl;
    http.begin(requestUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("HTTP Response: " + payload);

      // 解析JSON响应
      const size_t capacity = JSON_OBJECT_SIZE(10) + 1024;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonObject result = doc["result"][0];
        if (result.containsKey("dapandata")) {
          JsonObject dapandata = result["dapandata"];
          
          stockData.name = dapandata["name"].as<String>();
          stockData.code = stockCode;
          stockData.price = dapandata["dot"].as<String>();
          stockData.rate = dapandata["rate"].as<String>();
          stockData.change = dapandata["diff"].as<String>();
          stockData.open = dapandata["open"].as<String>();
          stockData.high = dapandata["high"].as<String>();
          stockData.low = dapandata["low"].as<String>();
          stockData.volume = dapandata["volume"].as<String>();
          stockData.time = "15:00:00";
          stockData.isValid = true;

          Serial.println("真实数据获取成功");
        }
      } else {
        Serial.println("JSON解析失败: " + String(error.c_str()));
      }
    } else {
      Serial.println("获取数据失败，HTTP状态码: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi未连接");
  }

  // 如果真实数据获取失败，使用模拟数据
  if (!stockData.isValid) {
    Serial.println("使用模拟股票数据");
    stockData = generateMockStockData();
  }

  // 打印股票信息
  printStockData(stockData);

  // 在墨水屏上显示数据
  Serial.println("开始在墨水屏上显示数据...");
  si_stock_screen(stockData);

  // 等待屏幕刷新完成
  int timeout = 30; // 30秒超时
  while (si_screen_status() == 0 && timeout > 0) {
    delay(1000);
    timeout--;
    Serial.print(".");
  }
  
  if (si_screen_status() == 1) {
    Serial.println("屏幕更新完成！");
  } else {
    Serial.println("屏幕更新超时");
  }
}

// 显示模拟股票数据
void displayMockStockData() {
  Serial.println("显示模拟股票数据");
  
  StockData mockData = generateMockStockData();
  printStockData(mockData);
  
  Serial.println("在墨水屏上显示模拟数据...");
  si_stock_screen(mockData);

  // 等待屏幕刷新完成
  int timeout = 30;
  while (si_screen_status() == 0 && timeout > 0) {
    delay(1000);
    timeout--;
    Serial.print(".");
  }
  
  if (si_screen_status() == 1) {
    Serial.println("模拟数据显示完成！");
  } else {
    Serial.println("模拟数据显示超时");
  }
}
// 保存参数前的回调
void preSaveParamsCallback() {
  // 可在此处添加保存前的逻辑
}

// 保存参数后的回调
void saveParamsCallback() {
  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  pref.putString(PREF_STOCK_API_KEY, para_stock_api_key.getValue());
  pref.putString(PREF_STOCK_CODE, para_stock_code.getValue());
  pref.end();

  Serial.println("参数已保存");
  Serial.println("API Key: " + String(para_stock_api_key.getValue()));
  Serial.println("Stock Code: " + String(para_stock_code.getValue()));

  delay(1000);
  ESP.restart(); // 成功后重启
}

// 配置WiFiManager
void setupWiFiManager() {
  Serial.println("正在进入配置模式");
  if (wm.getConfigPortalActive()) {
    ESP.restart();
    return;
  }

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  String stockApiKey = pref.getString(PREF_STOCK_API_KEY);
  String stockCode = pref.getString(PREF_STOCK_CODE);
  pref.end();

  para_stock_api_key.setValue(stockApiKey.c_str(), 64);
  para_stock_code.setValue(stockCode.c_str(), 32);

  wm.setTitle("Stock Config");
  wm.addParameter(&para_stock_api_key);
  wm.addParameter(&para_stock_code);
  std::vector<const char*> menu = { "wifi", "param" };
  wm.setMenu(menu);
  wm.setConfigPortalBlocking(false);
  wm.setBreakAfterConfig(true);
  wm.setPreSaveParamsCallback(preSaveParamsCallback);
  wm.setSaveParamsCallback(saveParamsCallback);
  wm.setSaveConnect(false);
  wm.startConfigPortal("keke-stock", "password");
}

// =====================
// 主程序
// =====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  wm.setHostname("keke-stock");
  wm.setEnableConfigPortal(true);
  wm.setConnectTimeout(30);

  setupWiFiManager();

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  String stockApiKey = pref.getString(PREF_STOCK_API_KEY);
  String stockCode = pref.getString(PREF_STOCK_CODE);
  pref.end();

  Serial.println("获取到的Stock API Key: " + stockApiKey);
  Serial.println("获取到的Stock Code: " + stockCode);

  if (wm.autoConnect()) {
    _wifi_flag = true;
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // 初始显示股票数据（尝试真实数据，失败则使用模拟数据）
    fetchAndDisplayStockData();
  } else {
    _wifi_flag = false;
    Serial.println("Failed to connect or config portal timeout.");
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi closed.");
    
    // 无WiFi连接时显示模拟数据
    displayMockStockData();
  }
}

void loop() {
  wm.process();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (_wifi_flag && WiFi.status() == WL_CONNECTED) {
      // 使用带显示功能的股票数据获取
      fetchAndDisplayStockData();
    } else {
      // 无网络时显示模拟数据
      displayMockStockData();
    }
  }
}

