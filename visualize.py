import pandas as pd
import matplotlib.pyplot as plt

print("📊 開始生成模擬結果圖表...")

# 1. 讀取剛剛 C++ 產出的 CSV 檔案
grid_df = pd.read_csv("clean_data/grid_blind_spots.csv")
station_df = pd.read_csv("clean_data/station_bottlenecks.csv")

# 2. 排序並過濾出 Top 10 最嚴重的區域與分隊
top_grids = grid_df.sort_values(by="Timeout_Count", ascending=False).head(10)
top_stations = station_df.sort_values(by="Shortage_Count", ascending=False).head(10)

# 3. 設定畫布大小 (寬 12, 高 5)
plt.figure(figsize=(12, 5))

# --- 子圖 1: 區域盲區 (紅色系) ---
plt.subplot(1, 2, 1)
# 將 Grid_ID 轉成字串，避免 X 軸把它當作連續數字來畫
plt.bar(top_grids["Grid_ID"].astype(str), top_grids["Timeout_Count"], color="salmon")
plt.title("Top 10 Grid Blind Spots (Timeout > 30 mins)")
plt.xlabel("Grid ID")
plt.ylabel("Timeout Frequency")

# --- 子圖 2: 分隊量能瓶頸 (藍色系) ---
plt.subplot(1, 2, 2)
plt.bar(top_stations["Station_ID"].astype(str), top_stations["Shortage_Count"], color="skyblue")
plt.title("Top Station Capacity Bottlenecks")
plt.xlabel("Station ID")
plt.ylabel("Shortage Frequency")

# 4. 自動排版並儲存成圖片
plt.tight_layout()
plt.savefig("clean_data/simulation_dashboard.png", dpi=300)
print("✅ 圖表生成完畢！已儲存至 clean_data/simulation_dashboard.png")

# 顯示在螢幕上
plt.show()