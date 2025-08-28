#include "stock_data.h"

// 模拟股票数据生成
StockData generateMockStockData() {
    StockData mockData;
    
    // 模拟不同的股票数据
    String stockNames[] = {"平安银行", "南京银行", "招商银行", "工商银行", "建设银行"};
    String stockCodes[] = {"000001", "601009", "600036", "601398", "601939"};
    
    // 随机选择一只股票
    int index = random(0, 5);
    mockData.code = stockCodes[index];
    mockData.name = stockNames[index];
    
    // 生成随机价格数据
    float basePrice = 10.0 + random(0, 200) / 10.0; // 10.0 到 30.0 之间
    mockData.price = String(basePrice, 2);
    
    // 生成涨跌幅（-10% 到 +10%）
    float rateValue = (random(-1000, 1000) / 100.0);
    mockData.rate = String(rateValue, 2);
    
    // 生成涨跌额
    float changeValue = basePrice * rateValue / 100.0;
    mockData.change = String(changeValue, 2);
    
    // 生成其他价格数据
    mockData.open = String(basePrice + random(-100, 100) / 100.0, 2);
    mockData.high = String(basePrice + random(0, 200) / 100.0, 2);
    mockData.low = String(basePrice - random(0, 200) / 100.0, 2);
    
    // 生成成交量（万手）
    mockData.volume = String(random(1000, 50000)) + "万手";
    
    // 生成时间戳
    mockData.time = "15:00:00";
    
    mockData.isValid = true;
    
    return mockData;
}

// 打印股票数据到串口
void printStockData(const StockData& data) {
    if (!data.isValid) {
        Serial.println("股票数据无效");
        return;
    }
    
    Serial.println("=== 股票信息 ===");
    Serial.println("名称: " + data.name);
    Serial.println("代码: " + data.code);
    Serial.println("价格: " + data.price + " 元");
    Serial.println("涨跌幅: " + data.rate + "%");
    Serial.println("涨跌额: " + data.change + " 元");
    Serial.println("开盘价: " + data.open + " 元");
    Serial.println("最高价: " + data.high + " 元");
    Serial.println("最低价: " + data.low + " 元");
    Serial.println("成交量: " + data.volume);
    Serial.println("更新时间: " + data.time);
    Serial.println("===============");
}
