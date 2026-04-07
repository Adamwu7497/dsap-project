import pandas as pd
import subprocess
import os

# 建立一個資料夾來存放清洗後的乾淨檔案
os.makedirs('clean_data', exist_ok=True)

print("⏳ 開始清洗【車程矩陣】資料...")
# 1. 使用 read_excel，並透過 sheet_name 指定要讀取的分頁
df_travel = pd.read_excel('GoogleMapsAPI_凌晨三點行車時間.xlsx', sheet_name='分隊到網格中心')

# 2. 使用 melt 將寬表格轉換成長表格
# 這裡注意：Excel 檔第一欄的名字可能叫「分隊」或「Unnamed: 0」，依據你的檔案實際狀況，
# 如果報錯說找不到 '分隊'，請把下面這行的 '分隊' 換成你 Excel 第一欄真正的名字
df_travel_long = df_travel.melt(id_vars=['分隊'], var_name='網格編號', value_name='車程時間')

# 3. 確保網格編號是整數
df_travel_long['網格編號'] = df_travel_long['網格編號'].astype(int)

# 4. 匯出給 C++ 用的乾淨版本 (匯出時還是存成 CSV，因為 C++ 讀 CSV 最快)
df_travel_long.to_csv('clean_data/clean_station_to_grid.csv', index=False)
print("✅ 車程矩陣清洗完成！已儲存至 clean_data/clean_station_to_grid.csv\n")


print("⏳ 開始清洗【500 筆測試案件】資料...")
# 5. 改讀 500 筆案件的 Excel 檔
df_cases = pd.read_excel('案件_500_不含精確地址.xlsx')

# 6. 篩選出「有效案件」
df_cases_clean = df_cases[df_cases['有效/取消案件'] == '有效案件'].copy()

# 7. 抓出模擬器需要的核心欄位
keep_columns = ['案號', '案發時間', '案件地點網格編號']
df_cases_clean = df_cases_clean[keep_columns]

# 8. 處理缺失值 (Drop掉沒有網格編號的資料)
df_cases_clean = df_cases_clean.dropna(subset=['案件地點網格編號', '案發時間'])

# 9. 將「案發時間」統一轉為時間物件，再轉為容易讓 C++ 排序的 Unix Timestamp (整數)
df_cases_clean['案發時間'] = pd.to_datetime(df_cases_clean['案發時間'])
df_cases_clean['timestamp'] = df_cases_clean['案發時間'].astype('int64') // 10**9 

# 10. 依照時間先後順序排序
df_cases_clean = df_cases_clean.sort_values(by='timestamp')

# 11. 匯出乾淨案件 (特別標註這是 500 筆的版本)
df_cases_clean.to_csv('clean_data/clean_cases_500.csv', index=False)
print("✅ 500 筆案件資料清洗完成！已儲存至 clean_data/clean_cases_500.csv\n")


# ==========================================
# GitHub 自動同步區塊
# ==========================================
print("🚀 開始自動同步至 GitHub (Adamwu7497/dsap-project)...")

try:
    subprocess.run(["git", "add", "."], check=True)
    subprocess.run(["git", "commit", "-m", "Auto-update cleaned 500 cases data via Python"], check=False)
    subprocess.run(["git", "push"], check=True)
    print("🎉 成功推送到 GitHub Repo！")
except subprocess.CalledProcessError as e:
    print(f"⚠️ 推送 GitHub 時發生錯誤，請確認設定：{e}")

