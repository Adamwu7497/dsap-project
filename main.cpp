#include <iostream>
#include <queue>
#include <vector>
#include "models.h"
#include "TravelTimeTable.h"
#include <set>

using namespace std;

// 建立全域的效能評估追蹤器
SimulationResult sim_results;

#include <fstream>
#include <sstream>

// ==========================================
// 讀取 25 萬筆真實案件並塞入時間軸 (Min-Heap)
// ==========================================
bool load_cases_from_csv(string filepath, priority_queue<Event>& eq, vector<Case*>& all_cases) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "❌ 錯誤：找不到案件檔案 " << filepath << "\n";
        return false;
    }

    string line;
    bool is_header = true;
    int count = 0;

    while (getline(file, line)) {
        if (is_header) { 
            is_header = false; // 跳過第一行標題
            continue; 
        }

        stringstream ss(line);
        // 準備四個字串來接你截圖裡的四個欄位
        string id_str, datetime_str, grid_str, ts_str;

        // 🎯 嚴格依照真實 CSV 欄位順序切割：
        getline(ss, id_str, ',');       // 1. 案號
        getline(ss, datetime_str, ','); // 2. 案發時間 (字串，我們略過不用)
        getline(ss, grid_str, ',');     // 3. 案件地點網格編號
        getline(ss, ts_str, ',');       // 4. timestamp (核心時間軸)

        // 確保核心資料沒有空缺
        if (!id_str.empty() && !ts_str.empty() && !grid_str.empty()) {
            long long ts = stoll(ts_str); // 字串轉長整數 (Unix Timestamp)
            int grid = stoi(grid_str);    // 字串轉整數

            // 建立案件物件並存在全域陣列中 (避免記憶體遺失)
            Case* new_case = new Case(id_str, ts, grid);
            all_cases.push_back(new_case);

            // 將案件轉化為「報案事件」，直接丟入 Min-Heap 時間軸！
            eq.push(Event(ts, CASE_OCCUR, new_case, nullptr));
            count++;
        }
    }
    file.close();
    cout << "✅ 成功將 " << count << " 筆案件載入時間軸引擎！\n";
    return true;
}

void auto_build_stations(string filepath, unordered_map<int, Station*>& all_stations) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "❌ 錯誤：找不到車程檔案 " << filepath << "\n";
        return;
    }

    string line;
    bool is_header = true;
    set<string> unique_stations;

    // 1. 掃描整個 CSV，萃取所有不重複的分隊名稱
    while (getline(file, line)) {
        if (is_header) { is_header = false; continue; }
        stringstream ss(line);
        string station_name;
        
        // 假設你的 clean_station_to_grid.csv 第一欄是「分隊名稱」
        getline(ss, station_name, ',');
        
        if (!station_name.empty()) {
            unique_stations.insert(station_name); // set 會自動過濾掉重複的名字
        }
    }
    file.close();

    // 🌟 新增：自動將 ID 與真實名字的對照表寫出成 CSV
    ofstream dict_file("clean_data/station_dictionary.csv");
    if (dict_file.is_open()) {
        dict_file << "Station_ID,Station_Name\n"; // 欄位名稱
        for (auto& pair : all_stations) {
            dict_file << pair.first << "," << pair.second->name << "\n";
        }
        dict_file.close();
        cout << "📝 已自動生成分隊對照表：clean_data/station_dictionary.csv\n";
    }

    // 2. 幫每個找出來的分隊建置實體，並配發基準量能 (例如各 3 台車)
    int id_counter = 1;
    for (const string& s_name : unique_stations) {
        // new 一個新的分隊物件
        Station* new_station = new Station(id_counter, s_name);
        
        // 統一配發 3 台救護車給這個分隊
        for (int i = 1; i <= 3; i++) {
            string plate = s_name + "-" + to_string(i); // 例如：大安-1, 大安-2
            new_station->all_ambulances.push_back(Ambulance(plate, "一般", id_counter));
        }
        
        // 把這些剛出廠的救護車推入 std::set (紅黑樹) 車庫待命
        for (int i = 0; i < new_station->all_ambulances.size(); i++) {
            new_station->available_cars.insert(&(new_station->all_ambulances[i]));
        }
        
        all_stations[id_counter] = new_station;
        id_counter++;
    }
    cout << "✅ 城市救護網建置完畢！共建立 " << unique_stations.size() 
         << " 個分隊，配發總計 " << unique_stations.size() * 3 << " 台救護車。\n";
}

int main() {
    cout << "🚀 啟動台北市救護資源離散事件模擬器 (DES Engine)...\n";

    // ==========================================
    // 1. 系統初始化：載入地圖與建置分隊
    // ==========================================
    TravelTimeTable time_table;
    if (!time_table.load_csv("clean_data/clean_station_to_grid.csv")) {
        return 1; 
    }

    // 用一個 Hash Map 管理全台北市的分隊
    unordered_map<int, Station*> all_stations;
    
    // 🌟 一鍵自動萃取並建置全台北市的分隊！
    auto_build_stations("clean_data/clean_station_to_grid.csv", all_stations);


    // ==========================================
    // 2. 載入案件，轉化為「報案事件」塞入 Min-Heap
    // ==========================================
    // C++ 的 priority_queue 預設是 Max-Heap，但因為我們在 models.h 裡
    // 把 Event 的 < 運算子反過來寫了，所以這裡它會乖乖變成 Min-Heap！
    priority_queue<Event> event_queue;
    vector<Case*> global_cases; 

    // 呼叫我們量身打造的函數！
    if (!load_cases_from_csv("clean_data/clean_cases_250k.csv", event_queue, global_cases)) {
        return 1;
    }

    // (實務上這裡應該讀取 cases.csv，這裡先模擬兩筆案件)
    Case* c1 = new Case("CASE-001", 1000, 12); // 時間 1000, 網格 12
    Case* c2 = new Case("CASE-002", 1500, 45); // 時間 1500, 網格 45

    event_queue.push(Event(1000, CASE_OCCUR, c1, nullptr));
    event_queue.push(Event(1500, CASE_OCCUR, c2, nullptr));

    cout << "✅ 系統初始化完成，開始模擬 25 萬筆事件！\n\n";

    // ==========================================
    // 3. ⏱️ 模擬器主迴圈 (時間瞬間移動的秘密)
    // ==========================================
    long long current_time = 0;

    while (!event_queue.empty()) {
        // 拿出時間最接近的事件
        Event current_event = event_queue.top();
        event_queue.pop();

        // 系統時間直接「瞬間移動」到這個事件發生的當下！
        current_time = current_event.event_time;

        // ------------------------------------------
        // 情境 A：有人報案了！準備派車
        // ------------------------------------------
        // ------------------------------------------
        // 情境 A：有人報案了！準備尋找救護車
        // ------------------------------------------
        if (current_event.type == CASE_OCCUR) {
            Case* current_case = current_event.related_case;
            cout << "[時間 " << current_time << "] 🚨 案件發生在網格 " << current_case->grid_id << "\n";

            Station* ideal_station = nullptr;
            int ideal_travel_time = 999999;

            // 步驟 1：先找出地理上「應該要最快到」的首選分隊
            for (auto& pair : all_stations) {
                Station* st = pair.second;
                int t_time = time_table.get_time(st->name, current_case->grid_id);
                if (t_time < ideal_travel_time) {
                    ideal_travel_time = t_time;
                    ideal_station = st;
                }
            }

            // 步驟 2：檢查首選分隊有沒有車
            Ambulance* dispatched_car = nullptr;
            Station* dispatching_station = nullptr;
            int actual_travel_time = 999999;

            if (ideal_station != nullptr) {
                dispatched_car = ideal_station->get_available_ambulance(current_time);
                
                if (dispatched_car != nullptr) {
                    // 太棒了！首選分隊有車
                    dispatching_station = ideal_station;
                    actual_travel_time = ideal_travel_time;
                } else {
                    // ❌ 慘了，首選分隊沒車！記錄瓶頸並觸發「跨區調派」
                    cout << "   ❌ 瓶頸發生：最近的分隊 (" << ideal_station->name << ") 無車可用！\n";
                    sim_results.record_station_bottleneck(ideal_station->station_id);

                    // 步驟 3：尋找全台北市「目前有車」且「距離次近」的分隊
                    for (auto& pair : all_stations) {
                        Station* st = pair.second;
                        Ambulance* backup_car = st->get_available_ambulance(current_time);
                        if (backup_car != nullptr) {
                            int t_time = time_table.get_time(st->name, current_case->grid_id);
                            if (t_time < actual_travel_time) {
                                actual_travel_time = t_time;
                                dispatching_station = st;
                                dispatched_car = backup_car;
                            }
                        }
                    }
                }
            }

            // 步驟 4：正式派車與紀錄時間
            if (dispatched_car != nullptr && dispatching_station != nullptr) {
                dispatching_station->dispatch_ambulance(dispatched_car);
                current_case->wait_time = actual_travel_time;
                
                cout << "   -> 派出 " << dispatching_station->name << " 分隊救護車 " 
                     << dispatched_car->plate_number << "，預估 " << actual_travel_time << " 分鐘後抵達。\n";

                // 📊 評估盲區指標
                if (current_case->wait_time > OPTIMAL_RESPONSE_TIME) {
                    sim_results.record_blind_spot(current_case->grid_id);
                    cout << "   ⚠️ 警告：該案件等候超時 (" << current_case->wait_time << " 分鐘)\n";
                }

                // 預估歸隊時間 (車程 + 現場處理與送醫假設 60 分鐘)
                long long return_time = current_time + actual_travel_time + 60;
                event_queue.push(Event(return_time, AMBULANCE_RETURN, nullptr, dispatched_car));
            } else {
                cout << "   🚨 嚴重危機：全台北市完全無救護車可用！\n";
            }
        }
        // ------------------------------------------
        // 情境 B：救護車任務結束，回歸車庫
        // ------------------------------------------
        else if (current_event.type == AMBULANCE_RETURN) {
            Ambulance* returning_car = current_event.related_ambulance;
            Station* home_station = all_stations[returning_car->station_id];

            // 呼叫 Week 4 寫好的紅黑樹歸隊函數，它會自動排好隊
            home_station->return_ambulance(returning_car, current_time);
            
            cout << "[時間 " << current_time << "] 🔄 救護車 " << returning_car->plate_number 
                 << " 已歸隊，重新加入待命序列。\n";
        }
    }

    // ==========================================
    // 4. 模擬結束，產出報告
    // ==========================================
    // ==========================================
    // 4. 模擬結束，產出報告與匯出資料
    // ==========================================
    sim_results.print_summary();
    sim_results.export_to_csv(); // 🌟 加上這行！

    // ==========================================
    // 5. 系統關閉與記憶體清理 (Memory Management)
    // ==========================================
    cout << "\n🧹 模擬結束，正在釋放 25 萬筆案件的記憶體...\n";
    for (Case* c : global_cases) {
        delete c; // 把 new 出來的空間還給作業系統
    }
    global_cases.clear();

    // 🌟 新增：清空所有自動建置的分隊
    for (auto& pair : all_stations) {
        delete pair.second;
    }
    all_stations.clear();

    cout << "✅ 記憶體釋放完畢！系統安全關閉。\n";


    return 0;
}