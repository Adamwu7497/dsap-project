#include <iostream>
#include "models.h"

using namespace std;

int main() {
    cout << "🏥 啟動 Week 4: 動態車庫 (std::set) 排序測試...\n\n";

    // 1. 建立一個測試分隊
    Station station(1, "建國");
    cout << "✅ 建立分隊：" << station.name << "\n";

    // 2. 建立三台救護車並放入分隊的「實體陣列」中
    station.all_ambulances.push_back(Ambulance("AAA-001", "一般", 1));
    station.all_ambulances.push_back(Ambulance("BBB-002", "一般", 1));
    station.all_ambulances.push_back(Ambulance("CCC-003", "高級", 1));

    // 3. 初始狀態：把三台車的「指標」都塞進紅黑樹 (available_cars) 裡排隊
    for (int i = 0; i < station.all_ambulances.size(); i++) {
        station.available_cars.insert(&station.all_ambulances[i]);
    }
    cout << "✅ 初始狀態：3 台救護車已在車庫待命。\n\n";

    // 4. 模擬案件發生，拿取最快的車
    long long current_time = 1000; 
    Ambulance* car1 = station.get_available_ambulance(current_time);
    if (car1 != nullptr) {
        cout << "🚨 案件 1 發生！派出救護車：" << car1->plate_number << "\n";
        station.dispatch_ambulance(car1);
    }

    Ambulance* car2 = station.get_available_ambulance(current_time);
    if (car2 != nullptr) {
        cout << "🚨 案件 2 發生！派出救護車：" << car2->plate_number << "\n";
        station.dispatch_ambulance(car2);
    }

    // 🌟 多加這段：把 CCC 也派出去，淨空車庫
    Ambulance* car3_initial = station.get_available_ambulance(current_time);
    if (car3_initial != nullptr) {
        cout << "🚨 案件 3 發生！派出救護車：" << car3_initial->plate_number << "\n";
        station.dispatch_ambulance(car3_initial);
    }

    cout << "   目前車庫剩下 " << station.available_cars.size() << " 台車\n\n";

    // 5. 模擬救護車歸隊 (見證奇蹟的時刻：Set 自動排序)
    cout << "🔄 救護車開始歸隊...\n";
    
    // 假設第二台車 (BBB-002) 比較快完成任務，在時間 2500 可以再次出勤
    station.return_ambulance(car2, 2500); 
    cout << "   " << car2->plate_number << " 歸隊，下次可用時間：2500\n";

    // 假設第一台車 (AAA-001) 遇到大塞車，要到時間 4000 才能再次出勤
    station.return_ambulance(car1, 4000);
    cout << "   " << car1->plate_number << " 歸隊，下次可用時間：4000\n\n";

    // 6. 再次派車，驗證 set 是否有把時間早的排在前面
    Ambulance* car4 = station.get_available_ambulance(5000);
    cout << "🚨 案件 4 發生！紅黑樹自動挑選最快可用的車是：" << car4->plate_number << "\n";

    // 驗證邏輯
    if (car4->plate_number == "BBB-002") {
        cout << "\n🎉 測試成功！std::set 完美發揮了 O(log N) 的動態排序功能！\n";
    } else {
        cout << "\n⚠️ 排序似乎有問題，請檢查 models.h 裡的 AmbulanceCompare 邏輯。\n";
    }

    return 0;
}