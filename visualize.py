import pandas as pd
import matplotlib.pyplot as plt

print("📊 開始生成模擬結果圖表...")

# ==========================================
# 1. 讀取數據與分隊密碼本
# ==========================================
grid_df = pd.read_csv("clean_data/grid_blind_spots.csv")
station_df = pd.read_csv("clean_data/station_bottlenecks.csv")
station_dict = pd.read_csv("clean_data/station_dictionary.csv")

# 透過 Station_ID 把真實分隊名稱串接起來 (類似 SQL 的 JOIN)
station_df = pd.merge(station_df, station_dict, on="Station_ID")

# 排序出 Top 10
top_grids = grid_df.sort_values(by="Timeout_Count", ascending=False).head(10)
top_stations = station_df.sort_values(by="Shortage_Count", ascending=False).head(10)

# ==========================================
# 2. 🌟 神奇解碼器：將一維 Grid ID 還原成二維空間座標
# ==========================================
def decode_grid(grid_id):
    # 根據我們推測出的寬度 30 進行反向運算
    y = grid_id // 30
    x = grid_id % 30
    return f"Grid({x},{y})"

# 幫網格資料表新增一個「真實座標名字」的欄位
top_grids["Grid_Name"] = top_grids["Grid_ID"].apply(decode_grid)

# ==========================================
# 3. 繪製圖表 (包含 Mac 中文字型設定)
# ==========================================
# 解決 Mac 顯示中文變方塊的問題
plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']
plt.rcParams['axes.unicode_minus'] = False

plt.figure(figsize=(12, 5))

# --- 子圖 1: 區域盲區 ---
plt.subplot(1, 2, 1)
# X 軸換成我們解碼出來的 (X, Y) 座標
plt.bar(top_grids["Grid_Name"], top_grids["Timeout_Count"], color="salmon")
plt.title("Top 10 區域盲區 (Timeout > 30 mins)")
plt.xlabel("網格座標 (X, Y)")
plt.ylabel("超時次數")
plt.xticks(rotation=45) # 字體轉 45 度避免擠在一起

# --- 子圖 2: 分隊量能瓶頸 ---
plt.subplot(1, 2, 2)
# X 軸換成真實的中文分隊名稱
plt.bar(top_stations["Station_Name"], top_stations["Shortage_Count"], color="skyblue")
plt.title("Top 分隊量能瓶頸")
plt.xlabel("分隊名稱")
plt.ylabel("缺車次數")

# ==========================================
# 4. 輸出與儲存
# ==========================================
plt.tight_layout()
plt.savefig("clean_data/simulation_dashboard_final.png", dpi=300)
print("✅ 圖表生成完畢！已儲存至 clean_data/simulation_dashboard_final.png")

plt.show()