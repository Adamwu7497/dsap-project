#include <iostream>
#include "TravelTimeTable.h"
#include "models.h" // 順便把上週的骨架拉進來測試

using namespace std;

int main() {
    cout << "🚑 啟動台北市救護資源模擬器測試環境...\n";

    // 實例化我們的車程資料庫
    TravelTimeTable time_table;

    // 載入 Week 1 洗好的乾淨資料
    // 請確認路徑與檔名正確，這裡假設放在 clean_data 資料夾下
    if (!time_table.load_csv("clean_data/clean_station_to_grid.csv")) {
        return 1; // 載入失敗就結束程式
    }

    cout << "\n--- 🔍 任意門 O(1) 查詢測試 ---\n";
    
    // 隨便找幾個你 Excel 裡面的真實分隊與網格來查查看
    string test_station = "忠孝"; 
    int test_grid = 12; // 這裡請替換成你資料庫裡確實存在的網格數字

    int time_cost = time_table.get_time(test_station, test_grid);
    
    if (time_cost != 999999) {
        cout << test_station << " 分隊到網格 " << test_grid 
             << " 的預估車程為： " << time_cost << " 分鐘\n";
    } else {
        cout << "⚠️ 查無此路線的行車時間！\n";
    }

    return 0;
}