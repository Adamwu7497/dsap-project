#ifndef TRAVEL_TIME_TABLE_H
#define TRAVEL_TIME_TABLE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility> // 為了使用 std::pair

using namespace std;

// ==========================================
// 自定義的 Hash 函數：教 C++ 如何把 pair<string, int> 變成 Hash 值
// ==========================================
struct PairHash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first);  // 計算字串的 Hash
        auto h2 = hash<T2>{}(p.second); // 計算數字的 Hash
        return h1 ^ (h2 << 1);          // 混合兩個 Hash 值
    }
};

// ==========================================
// 車程資料庫 Class
// ==========================================
class TravelTimeTable {
private:
    // 核心資料結構：Key 是 <分隊名稱, 網格編號>, Value 是 車程時間(秒或分)
    unordered_map<pair<string, int>, int, PairHash> table;

public:
    // 讀取 CSV 並建立 Hash Table
    bool load_csv(string filepath) {
        ifstream file(filepath);
        if (!file.is_open()) {
            cerr << "錯誤：找不到檔案 " << filepath << "\n";
            return false;
        }

        string line;
        bool is_header = true;

        // 一行一行讀取檔案
        while (getline(file, line)) {
            if (is_header) { 
                is_header = false; // 跳過第一行的標題
                continue; 
            }

            stringstream ss(line);
            string station_name, grid_str, time_str;

            // 以逗號分隔切割字串
            getline(ss, station_name, ',');
            getline(ss, grid_str, ',');
            getline(ss, time_str, ',');

            // 確保資料沒有空缺
            if (!station_name.empty() && !grid_str.empty() && !time_str.empty()) {
                int grid_id = stoi(grid_str);
                int travel_time = stoi(time_str);

                // 存入 Hash Table！ (這就是 O(1) 的威力來源)
                table[{station_name, grid_id}] = travel_time;
            }
        }
        file.close();
        cout << "✅ 成功載入車程資料庫！共讀取了 " << table.size() << " 筆行車時間。\n";
        return true;
    }

    // 查詢函數 (O(1) 瞬間回傳)
    int get_time(string station_name, int grid_id) {
        auto it = table.find({station_name, grid_id});
        if (it != table.end()) {
            return it->second; // 找到了，回傳時間
        } else {
            // 如果查不到 (極端邊界狀況)，回傳一個很大的數字代表無法抵達
            return 999999; 
        }
    }
};

#endif