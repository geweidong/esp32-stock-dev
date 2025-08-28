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
// 函数声明
// =====================
void fetchStockData();
void preSaveParamsCallback();
void saveParamsCallback();
void setupWiFiManager();

// =====================
// 函数实现
// =====================
// 获取股票数据
void fetchStockData() {
  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  String stockApiKey = pref.getString(PREF_STOCK_API_KEY);
  String stockCode = pref.getString(PREF_STOCK_CODE);
  pref.end();

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String requestUrl = apiUrl;
    http.begin(requestUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response payload: " + payload);

      // 使用ArduinoJson解析JSON响应
      const size_t capacity = JSON_OBJECT_SIZE(3) + 60; // 根据实际JSON结构调整容量
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        // 提取字段并打印
        JsonObject dapandata = doc["result"][0]["dapandata"].as<JsonObject>();
        String name = dapandata["name"].as<String>();
        String price = dapandata["dot"].as<String>();
        String rate = dapandata["rate"].as<String>();

        Serial.println("名称: " + name);
        Serial.println("价格: " + price);
        Serial.println("涨跌幅: " + rate + "%");
      } else {
        Serial.println("JSON Parse Error: " + String(error.c_str()));
      }
    } else {
      Serial.println("Error on HTTP request: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
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
    fetchStockData();
  } else {
    _wifi_flag = false;
    Serial.println("Failed to connect or config portal timeout.");
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi closed.");
  }
}

void loop() {
  wm.process();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (_wifi_flag && WiFi.status() == WL_CONNECTED) {
      fetchStockData();
    }
  }
}

