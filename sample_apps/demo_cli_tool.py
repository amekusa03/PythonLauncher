#!/usr/bin/env python3
import time
import sys

def main():
    print("==========================================", flush=True)
    print(" バックグラウンド継続型 サンプル (CLIツール)", flush=True)
    print("==========================================", flush=True)
    print("親アプリを終了しても、このスクリプトはバックグラウンドで動き続けます。", flush=True)
    
    count = 1
    while True:
        print(f"[{count}] バックグラウンド定期処理を実行中... (2秒ごとに記録)", flush=True)
        if count % 5 == 0:
            print(f"[NOTICE] チェックポイント {count}: 正常稼働中", file=sys.stderr, flush=True)
        time.sleep(2)
        count += 1

if __name__ == "__main__":
    main()
