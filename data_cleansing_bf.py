import pandas as pd
import subprocess
import os

# 確保輸出資料夾存在
os.makedirs('clean_data', exist_ok=True)

# (車程矩陣因為已經洗過並上傳了，為了節省時間，我們把這段註解掉或刪掉)
# print("⏳ 車程矩陣已清洗完畢，跳過...")

print("⏳ 開始清洗【25 萬筆完整案件】資料... (讀取大型 Excel 檔需要約 1-2 分鐘，請耐心等候☕)")

# 1. 讀取 25 萬筆案件的 Excel 檔 
# (請確認你資料夾裡的檔名真的是這個，如果有出入請自行微調)
df_cases = pd.read_excel('案件_252505_不含精確地址.xlsx')

# 2. 篩選出「有效案件」
df_cases_clean = df_cases[df_cases['有效/取消案件'] == '有效案件'].copy()

# 3. 抓出模擬器需要的核心欄位
keep_columns = ['案號', '案發時間', '案件地點網格編號']
df_cases_clean = df_cases_clean[keep_columns]

# 4. 處理缺失值 (Drop掉沒有網格編號的資料)
df_cases_clean = df_cases_clean.dropna(subset=['案件地點網格編號', '案發時間'])

# 5. 將「案發時間」統一轉為時間物件，再轉為容易讓 C++ 排序的 Unix Timestamp (整數)
df_cases_clean['案發時間'] = pd.to_datetime(df_cases_clean['案發時間'])
df_cases_clean['timestamp'] = df_cases_clean['案發時間'].astype('int64') // 10**9 

# 6. 依照時間先後順序排序
df_cases_clean = df_cases_clean.sort_values(by='timestamp')

# 7. 匯出乾淨案件 (標註為 250k 版本)
df_cases_clean.to_csv('clean_data/clean_cases_250k.csv', index=False)
print("✅ 25 萬筆案件資料清洗完成！已儲存至 clean_data/clean_cases_250k.csv\n")


# ==========================================
# GitHub 自動同步區塊
# ==========================================
print("🚀 開始自動同步至 GitHub (Adamwu7497/dsap-project)...")

try:
    subprocess.run(["git", "add", "."], check=True)
    subprocess.run(["git", "commit", "-m", "Auto-update cleaned 250k cases data"], check=False)
    subprocess.run(["git", "push"], check=True)
    print("🎉 成功推送到 GitHub Repo！25 萬筆大資料已經上雲端囉！")
except subprocess.CalledProcessError as e:
    print(f"⚠️ 推送 GitHub 時發生錯誤：{e}")