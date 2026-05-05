#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

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

    void print_summary() {
        cout << "\n📊 --- 模擬結果報告 --- 📊\n";
        cout << "正在分析 25 萬筆數據中的盲區與瓶頸...\n";
    }
};

#endif