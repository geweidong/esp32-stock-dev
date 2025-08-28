#ifndef __STOCK_DATA_H__
#define __STOCK_DATA_H__

#include <Arduino.h>

// 股票数据结构
struct StockData {
    String name;        // 股票名称
    String code;        // 股票代码
    String price;       // 当前价格
    String rate;        // 涨跌幅
    String change;      // 涨跌额
    String open;        // 开盘价
    String high;        // 最高价
    String low;         // 最低价
    String volume;      // 成交量
    String time;        // 更新时间
    bool isValid;       // 数据有效性
};

// 函数声明
StockData generateMockStockData();
void printStockData(const StockData& data);

#endif // __STOCK_DATA_H__
