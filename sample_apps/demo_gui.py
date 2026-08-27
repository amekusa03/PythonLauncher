#!/usr/bin/env python3
import tkinter as tk
from tkinter import messagebox
import sys

def main():
    print("Demo GUI app launched successfully!", flush=True)
    root = tk.Tk()
    root.title("サンプル GUI Python アプリ")
    root.geometry("420x260")
    root.configure(bg="#f4f6f9")
    
    lbl_title = tk.Label(root, text="Python GUI アプリケーション", font=("Helvetica", 14, "bold"), bg="#f4f6f9", fg="#2c3e50")
    lbl_title.pack(pady=(25, 5))

    lbl_desc = tk.Label(root, text="Qt C++ ランチャーからワンクリックで起動されました。\nターミナル操作なしで安全に実行できます。", font=("Helvetica", 10), bg="#f4f6f9", fg="#555555")
    lbl_desc.pack(pady=10)
    
    def on_click():
        messagebox.showinfo("動作確認", "Pythonスクリプトは正常に動作しています！")
        print("[Log] ユーザーがメッセージボタンを押しました。", flush=True)

    btn = tk.Button(root, text="メッセージを表示", command=on_click, font=("Helvetica", 11, "bold"), bg="#27ae60", fg="white", activebackground="#219150", activeforeground="white", padx=15, pady=8, bd=0, relief="flat")
    btn.pack(pady=15)
    
    root.mainloop()

if __name__ == "__main__":
    main()
