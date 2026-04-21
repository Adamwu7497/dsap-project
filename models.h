#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ==========================================
// 1. 案件 (Case)
// ==========================================
class Case {
public:
    string case_id;          // 案號 (例如 "20220101000328003")
    long long timestamp;     // 報案時間 (Unix Timestamp，從 CSV 讀取)
    int grid_id;             // 發生地點的網格編號

    // 模擬器運行後會填寫的數據 (用於期末數據分析)
    long long wait_time;     // 病患等待時間 (救護車抵達時間 - 報案時間)
    string assigned_plate;   // 派去處理的救護車車牌

    // 建構子 (Constructor)
    Case(string id, long long ts, int grid) {
        case_id = id;
        timestamp = ts;
        grid_id = grid;
        wait_time = -1;      // 初始化為 -1 代表尚未處理
        assigned_plate = "";
    }
};


// ==========================================
// 2. 救護車 (Ambulance)
// ==========================================
class Ambulance {
public:
    string plate_number;     // 車牌號碼
    string type;             // 基礎 or 高級
    int station_id;          // 所屬分隊的 ID 或名稱

    bool is_idle;            // 目前是否閒置 (true: 在分隊待命, false: 出勤中)
    long long available_time;// 下一次可以接案的時間 (Unix Timestamp)

    // 建構子
    Ambulance(string plate, string t, int s_id) {
        plate_number = plate;
        type = t;
        station_id = s_id;
        is_idle = true;      // 初始狀態都是閒置的
        available_time = 0;  
    }
};


// ==========================================
// 3. 分隊 (Station)
// ==========================================
class Station {
public:
    string name;             // 分隊名稱 (例如 "忠孝")
    int station_id;          // 分隊的數字 ID (方便陣列或 Hash Table 查找)
    
    // 裝載這個分隊所有救護車的陣列
    vector<Ambulance> all_ambulances; 

    // 建構子
    Station(int id, string n) {
        station_id = id;
        name = n;
    }

    // Week 4 才會實作的功能：從這裡面挑出一台閒置的車
    // Ambulance* get_available_ambulance(); 
};


// ==========================================
// 4. 事件 (Event) - 離散事件模擬的核心
// ==========================================
// 定義事件的三種類型
enum EventType {
    CASE_OCCUR,         // 報案發生
    AMBULANCE_RETURN    // 救護車完成任務歸隊
};

class Event {
public:
    long long event_time;    // 事件發生的時間點
    EventType type;          // 事件的種類

    // 指標：這個事件關聯到哪一個案件？哪一台車？
    // (報案事件會有 case_ptr；歸隊事件會有 amb_ptr)
    Case* related_case;      
    Ambulance* related_ambulance; 

    // 建構子
    Event(long long time, EventType t, Case* c_ptr, Ambulance* a_ptr) {
        event_time = time;
        type = t;
        related_case = c_ptr;
        related_ambulance = a_ptr;
    }

    // ✨ 超級重要：教 Priority Queue 怎麼排序事件
    // C++ 預設是 Max-Heap，我們希望「時間愈早的事件」排在愈前面 (Min-Heap)
    // 所以我們要在這裡覆寫小於 (<) 運算子，把邏輯反過來
    bool operator<(const Event& other) const {
        return event_time > other.event_time; 
    }
};

#endif