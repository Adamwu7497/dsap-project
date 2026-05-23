#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <fstream> // 🌟 補上這一行，讓系統認識 ofstream
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm> // 順便確保 sort 函數有被載入
using namespace std;

// ==========================================
// 1. 案件 (Case)
// ==========================================
class Case {
public:
    string case_id;          
    long long timestamp;     
    int grid_id;             

    long long wait_time;     
    string assigned_plate;   

    Case(string id, long long ts, int grid) {
        case_id = id;
        timestamp = ts;
        grid_id = grid;
        wait_time = -1;      
        assigned_plate = "";
    }
};

// ==========================================
// 2. 救護車 (Ambulance)
// ==========================================
class Ambulance {
public:
    string plate_number;     
    string type;             
    int station_id;          

    bool is_idle;            
    long long available_time;

    Ambulance(string plate, string t, int s_id) {
        plate_number = plate;
        type = t;
        station_id = s_id;
        is_idle = true;      
        available_time = 0;  
    }
};

// ==========================================
// 🌟 教 std::set 怎麼幫救護車排隊的規則
// ==========================================
struct AmbulanceCompare {
    bool operator()(const Ambulance* a, const Ambulance* b) const {
        if (a->available_time != b->available_time) {
            return a->available_time < b->available_time;
        }
        return a->plate_number < b->plate_number;
    }
};

// ==========================================
// 3. 分隊 (Station) - Week 4 升級版
// ==========================================
class Station {
public:
    string name;             
    int station_id;          
    
    vector<Ambulance> all_ambulances; 
    set<Ambulance*, AmbulanceCompare> available_cars;

    Station(int id, string n) {
        station_id = id;
        name = n;
    }

    Ambulance* get_available_ambulance(long long current_time) {
        if (available_cars.empty()) {
            return nullptr; 
        }
        Ambulance* best_car = *(available_cars.begin());
        return best_car;
    }

    void dispatch_ambulance(Ambulance* car) {
        car->is_idle = false;
        available_cars.erase(car); 
    }

    void return_ambulance(Ambulance* car, long long new_available_time) {
        car->is_idle = true;
        car->available_time = new_available_time;
        available_cars.insert(car); 
    }
};

// ==========================================
// 4. 事件 (Event) - 離散事件模擬的核心
// ==========================================
enum EventType {
    CASE_OCCUR,         
    AMBULANCE_RETURN    
};

class Event {
public:
    long long event_time;    
    EventType type;          
    Case* related_case;      
    Ambulance* related_ambulance; 

    Event(long long time, EventType t, Case* c_ptr, Ambulance* a_ptr) {
        event_time = time;
        type = t;
        related_case = c_ptr;
        related_ambulance = a_ptr;
    }

    // 讓 Priority Queue 知道時間越小越優先 (Min-Heap)
    bool operator<(const Event& other) const {
        return event_time > other.event_time; 
    }
};

// ==========================================
// 5. 模擬結果紀錄器 (Simulation Result)
// ==========================================
const int OPTIMAL_RESPONSE_TIME = 8; 

class SimulationResult {
public:
    unordered_map<int, int> grid_timeout_count;
    unordered_map<int, int> station_shortage_count;

    void record_blind_spot(int grid_id) {
        grid_timeout_count[grid_id]++;
    }

    void record_station_bottleneck(int station_id) {
        station_shortage_count[station_id]++;
    }

    // 模擬結束後，印出排名前幾名的問題網格與分隊
    void print_summary() {
        cout << "\n📊 ================= 模擬結果報告 ================= 📊\n";
        
        cout << "\n🚨 【區域盲區 Top 5】(即使派出最快的車仍超時的網格)：\n";
        // 簡單將 map 轉為 vector 以便排序
        vector<pair<int, int>> grids(grid_timeout_count.begin(), grid_timeout_count.end());
        sort(grids.begin(), grids.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.second > b.second; // 照超時次數由大排到小
        });
        
        int count = 0;
        for (auto& g : grids) {
            if (count++ >= 5) break;
            cout << "   網格 ID: " << g.first << " | 超時次數: " << g.second << " 次\n";
        }

        cout << "\n❌ 【分隊量能瓶頸】(因無車可用導致跨區調派的分隊)：\n";
        vector<pair<int, int>> stations(station_shortage_count.begin(), station_shortage_count.end());
        sort(stations.begin(), stations.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.second > b.second; 
        });
        
        for (auto& s : stations) {
            cout << "   分隊 ID: " << s.first << " | 缺車次數: " << s.second << " 次\n";
        }
        cout << "\n======================================================\n";
    }
    // 模擬結束後，將盲區與瓶頸數據匯出成 CSV 檔案
    void export_to_csv() {
        // 匯出網格盲區數據
        ofstream grid_file("clean_data/grid_blind_spots.csv");
        if (grid_file.is_open()) {
            grid_file << "Grid_ID,Timeout_Count\n"; // 寫入標題
            for (auto& pair : grid_timeout_count) {
                grid_file << pair.first << "," << pair.second << "\n";
            }
            grid_file.close();
            cout << "✅ 成功匯出區域盲區數據至 clean_data/grid_blind_spots.csv\n";
        }

        // 匯出分隊瓶頸數據
        ofstream station_file("clean_data/station_bottlenecks.csv");
        if (station_file.is_open()) {
            station_file << "Station_ID,Shortage_Count\n"; // 寫入標題
            for (auto& pair : station_shortage_count) {
                station_file << pair.first << "," << pair.second << "\n";
            }
            station_file.close();
            cout << "✅ 成功匯出分隊瓶頸數據至 clean_data/station_bottlenecks.csv\n";
        }
    }
};

#endif