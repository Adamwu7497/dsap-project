import pandas as pd
import folium

print("🗺️ 啟動地理座標轉換與地圖生成引擎...")

# ==========================================
# 1. 座標轉換參數設定 (請根據真實資料微調)
# ==========================================
# 假設網格原點 (Grid 0, X=0, Y=0) 位於台北市的西北角 (大約是北投/淡水交界)
START_LAT = 25.2100  # 起點緯度 (越往南 Y 越大，緯度越小)
START_LNG = 121.4500 # 起點經度 (越往東 X 越大，經度越大)

# 假設每個網格長寬大約是 1 公里 (經緯度大約跨越 0.009 度)
STEP_LAT = 0.009
STEP_LNG = 0.009

# 神奇解碼器：Grid ID -> 經緯度 (Lat, Lng)
def grid_to_latlng(grid_id):
    y = grid_id // 30
    x = grid_id % 30
    
    # 緯度往下(南)會遞減，經度往右(東)會遞增
    real_lat = START_LAT - (y * STEP_LAT)
    real_lng = START_LNG + (x * STEP_LNG)
    return real_lat, real_lng

# ==========================================
# 2. 讀取盲區數據
# ==========================================
grid_df = pd.read_csv("clean_data/grid_blind_spots.csv")
top_grids = grid_df.sort_values(by="Timeout_Count", ascending=False).head(10)

# ==========================================
# 3. 建立台北市底圖
# ==========================================
# 初始化地圖，中心點對準台北車站附近
m = folium.Map(location=[25.0478, 121.5170], zoom_start=12, tiles="CartoDB positron")

# ==========================================
# 4. 將盲區網格畫上地圖
# ==========================================
for index, row in top_grids.iterrows():
    grid_id = int(row["Grid_ID"])
    timeout_count = int(row["Timeout_Count"])
    
    # 呼叫轉換公式
    lat, lng = grid_to_latlng(grid_id)
    grid_x = grid_id % 30
    grid_y = grid_id // 30
    
    # 在地圖上畫紅色圈圈
    folium.CircleMarker(
        location=[lat, lng],
        radius=timeout_count / 150, # 根據超時次數決定圈圈大小 (可微調比例)
        color="red",
        fill=True,
        fill_color="red",
        fill_opacity=0.6,
        popup=f"<b>Grid ID:</b> {grid_id}<br><b>座標:</b> X:{grid_x}, Y:{grid_y}<br><b>超時次數:</b> {timeout_count}"
    ).add_to(m)

# ==========================================
# 5. 輸出成 HTML 互動網頁
# ==========================================
output_file = "clean_data/taipei_blind_spots_map.html"
m.save(output_file)
print(f"✅ 真實地圖生成完畢！請使用瀏覽器打開 {output_file} 查看。")