#!/usr/bin/env python3
import time
import sys

def main():
    print("==========================================", flush=True)
    print(" リアルタイム処理サンプル (CLIツール)", flush=True)
    print("==========================================", flush=True)
    
    total_steps = 10
    for i in range(1, total_steps + 1):
        percent = i * 10
        print(f"[{i}/{total_steps}] 処理を実行中... ({percent}%)", flush=True)
        if i == 5:
            print("[NOTICE] 中間データ同期完了", file=sys.stderr, flush=True)
        time.sleep(0.8)
        
    print("すべての処理が正常に完了しました！", flush=True)

if __name__ == "__main__":
    main()
