#include "i18n.h"
#include <QSettings>
#include <QLocale>

I18n::I18n(QObject *parent) : QObject(parent), m_currentLang(Language::Japanese) {
    initDictionary();

    QSettings settings("CustomTools", "PythonLauncher");
    int langVal = settings.value("language", -1).toInt();
    if (langVal == static_cast<int>(Language::English)) {
        m_currentLang = Language::English;
    } else if (langVal == static_cast<int>(Language::Japanese)) {
        m_currentLang = Language::Japanese;
    } else {
        // Default based on system locale
        QString locale = QLocale::system().name();
        if (locale.startsWith("ja", Qt::CaseInsensitive)) {
            m_currentLang = Language::Japanese;
        } else {
            m_currentLang = Language::English;
        }
    }
}

I18n& I18n::instance() {
    static I18n s_instance;
    return s_instance;
}

void I18n::setLanguage(Language lang) {
    if (m_currentLang != lang) {
        m_currentLang = lang;
        QSettings settings("CustomTools", "PythonLauncher");
        settings.setValue("language", static_cast<int>(lang));
        emit languageChanged(m_currentLang);
    }
}

QString I18n::t(const QString& key, const QStringList& args) const {
    const QMap<QString, QString>& dict = (m_currentLang == Language::English) ? m_dictEn : m_dictJa;
    QString result = dict.value(key, key);

    for (int i = 0; i < args.size(); ++i) {
        result.replace(QString("%%1").arg(i + 1), args[i]);
    }
    return result;
}

void I18n::initDictionary() {
    // --- Japanese Dictionary ---
    m_dictJa["app_window_title"] = "Python App Launcher";
    m_dictJa["app_title"] = "Python アプリ ランチャー";
    m_dictJa["app_subtitle"] = "コマンドライン入力不要・ワンクリックでPythonスクリプトを安全起動";
    m_dictJa["search_placeholder"] = "🔍 アプリ名で検索...";
    m_dictJa["btn_add_sample"] = "サンプルアプリ登録";
    m_dictJa["btn_add_app"] = "＋ 新規アプリ追加";
    m_dictJa["hint_drag_drop"] = "💡 ヒント: .py ファイルをこのウィンドウ内にドラッグ＆ドロップして簡単に登録できます。";
    m_dictJa["lang_select"] = "🌐 言語 / Language";

    // Desktop Shortcut Button
    m_dictJa["btn_desktop_register"] = "🖥️ アプリ一覧に登録";
    m_dictJa["tip_desktop_register"] = "Ubuntuのアプリケーション一覧にこのアプリを登録します";
    m_dictJa["btn_desktop_unregister"] = "🗑️ アプリ一覧から解除";
    m_dictJa["tip_desktop_unregister"] = "Ubuntuのアプリケーション一覧からこのアプリの登録を削除します";

    // Desktop Shortcut Messages
    m_dictJa["msg_desktop_reg_confirm_title"] = "登録解除の確認";
    m_dictJa["msg_desktop_reg_confirm_text"] = "Ubuntuのアプリケーション一覧から「Python Launcher」の登録を解除しますか？";
    m_dictJa["msg_desktop_unreg_success_title"] = "解除完了";
    m_dictJa["msg_desktop_unreg_success_text"] = "アプリケーション一覧から登録を解除しました。";
    m_dictJa["msg_desktop_unreg_failed"] = "ショートカットファイルの削除に失敗しました。";
    m_dictJa["msg_desktop_reg_success_title"] = "登録完了";
    m_dictJa["msg_desktop_reg_success_text"] = "Ubuntuのアプリケーション一覧に「Python Launcher」を登録しました！\n\n【起動方法】\n・画面左下の「アプリを表示」（またはSuperキー）から「Python Launcher」を検索して直接起動できます。";
    m_dictJa["msg_desktop_error_title"] = "エラー";
    m_dictJa["msg_desktop_dir_error"] = "アプリケーション格納フォルダを取得できませんでした。";
    m_dictJa["msg_desktop_create_error"] = "ショートカットファイルの作成に失敗しました:\n%1";

    // Card Items
    m_dictJa["card_badge_detached"] = "🔄 独立起動";
    m_dictJa["card_tip_detached"] = "親アプリ終了後もバックグラウンドで継続実行するモード";
    m_dictJa["card_status_running"] = "🟢 実行中 (PID: %1)";
    m_dictJa["card_status_starting"] = "🟡 起動処理中...";
    m_dictJa["card_status_failed"] = "🔴 起動失敗";
    m_dictJa["card_status_stopped"] = "⚪ 停止中";
    m_dictJa["card_no_desc"] = "（説明なし）";
    m_dictJa["card_btn_run"] = "▶ 起動";
    m_dictJa["card_btn_stop"] = "■ 停止";
    m_dictJa["card_btn_log"] = "📄 ログ";
    m_dictJa["card_tip_edit"] = "編集";
    m_dictJa["card_tip_delete"] = "削除";

    // Delete Confirmation
    m_dictJa["msg_delete_confirm_title"] = "登録削除";
    m_dictJa["msg_delete_confirm_text"] = "「%1」の登録を一覧から削除しますか？\n（スクリプトファイル自体は削除されません）";

    // Samples
    m_dictJa["sample_added_title"] = "サンプル登録";
    m_dictJa["sample_added_text"] = "動作テスト用のサンプルPythonアプリを2件追加しました！";
    m_dictJa["sample_gui_name"] = "サンプル GUI アプリ (Tkinter)";
    m_dictJa["sample_gui_desc"] = "ワンクリックで動くTkinterメッセージボックス付きサンプル画面";
    m_dictJa["sample_cli_name"] = "サンプル CLI ツール (リアルタイム出力)";
    m_dictJa["sample_cli_desc"] = "ログ画面にリアルタイム進捗とエラーを出力する処理サンプル";

    // Log Viewer
    m_dictJa["log_title"] = "📄 実行ログ モニター (ターミナル出力)";
    m_dictJa["log_target"] = "表示対象:";
    m_dictJa["log_all"] = "すべてのログ (全体)";
    m_dictJa["log_autoscroll"] = "自動スクロール";
    m_dictJa["log_clear"] = "ログ消去";
    m_dictJa["log_save"] = "ログ保存...";
    m_dictJa["log_save_dialog_title"] = "ログを保存";
    m_dictJa["log_save_filter"] = "ログファイル (*.log *.txt);;すべてのファイル (*)";
    m_dictJa["log_save_error_title"] = "保存失敗";
    m_dictJa["log_save_error_text"] = "ログファイルを書き込めませんでした。";
    m_dictJa["log_save_success_title"] = "保存完了";
    m_dictJa["log_save_success_text"] = "ログを保存しました: %1";

    // Add / Edit App Dialog
    m_dictJa["dlg_title_add"] = "新規Pythonアプリの登録";
    m_dictJa["dlg_title_edit"] = "Pythonアプリ設定の編集";
    m_dictJa["dlg_header_add"] = "新しいPythonアプリを登録します";
    m_dictJa["dlg_header_edit"] = "Pythonアプリの登録情報を編集します";
    m_dictJa["dlg_lbl_name"] = "アプリ名 (*):";
    m_dictJa["dlg_ph_name"] = "例: 画像一括リサイズツール";
    m_dictJa["dlg_lbl_desc"] = "説明:";
    m_dictJa["dlg_ph_desc"] = "例: ドラッグした画像を全自動で変換するスクリプト";
    m_dictJa["dlg_lbl_script"] = "スクリプトパス (*):";
    m_dictJa["dlg_ph_script"] = "例: /home/user/scripts/main.py";
    m_dictJa["dlg_btn_browse"] = "参照...";
    m_dictJa["dlg_lbl_interp"] = "Python環境 (Interp):";
    m_dictJa["dlg_ph_interp"] = "例: /usr/bin/python3 または venv/bin/python";
    m_dictJa["dlg_btn_autodetect"] = "自動検出";
    m_dictJa["dlg_lbl_workdir"] = "作業ディレクトリ:";
    m_dictJa["dlg_ph_workdir"] = "空欄の場合はスクリプトの場所";
    m_dictJa["dlg_lbl_args"] = "実行引数 (任意):";
    m_dictJa["dlg_ph_args"] = "例: --config settings.json --verbose";
    m_dictJa["dlg_lbl_icon"] = "アプリアイコン:";
    m_dictJa["dlg_ph_icon"] = "任意 (.png, .jpg, .ico, .svg)";
    m_dictJa["dlg_lbl_mode"] = "起動モード:";
    m_dictJa["dlg_chk_keepalive"] = "親アプリ終了後もバックグラウンドで継続実行する（独立起動）";
    m_dictJa["dlg_btn_cancel"] = "キャンセル";
    m_dictJa["dlg_btn_save"] = "保存";
    m_dictJa["dlg_btn_register"] = "登録";

    // Dialog File Browse titles
    m_dictJa["dlg_browse_script_title"] = "Pythonスクリプトを選択";
    m_dictJa["dlg_browse_script_filter"] = "Pythonファイル (*.py *.pyw);;すべてのファイル (*)";
    m_dictJa["dlg_browse_interp_title"] = "Python実行ファイルを選択";
    m_dictJa["dlg_browse_interp_filter"] = "実行可能ファイル (python python3);;すべてのファイル (*)";
    m_dictJa["dlg_browse_workdir_title"] = "作業ディレクトリを選択";
    m_dictJa["dlg_browse_icon_title"] = "アイコン画像を選択";
    m_dictJa["dlg_browse_icon_filter"] = "画像ファイル (*.png *.jpg *.jpeg *.ico *.svg);;すべてのファイル (*)";

    // Dialog Validation
    m_dictJa["dlg_val_error_title"] = "入力エラー";
    m_dictJa["dlg_val_name_empty"] = "アプリ名を入力してください。";
    m_dictJa["dlg_val_script_empty"] = "Pythonスクリプトのパスを指定してください。";

    // Process Runner Messages
    m_dictJa["proc_detached_start"] = "[システム] 独立起動(デタッチ)を開始します: %1 %2\n";
    m_dictJa["proc_detached_log_dest"] = "[システム] ログ出力先: %1\n";
    m_dictJa["proc_detached_bg_notice"] = "[システム] 親アプリを終了しても Python はバックグラウンドで動作し続けます。\n";
    m_dictJa["proc_detached_success"] = "[システム] 独立プロセスが正常に開始されました (PID: %1)\n";
    m_dictJa["proc_detached_failed"] = "[エラー] 独立プロセスの起動に失敗しました。\n";
    m_dictJa["proc_normal_start"] = "[システム] 起動中: %1 %2\n";
    m_dictJa["proc_send_sigterm"] = "[システム] 独立プロセス (PID: %1) にSIGTERMを送ります...\n";
    m_dictJa["proc_req_stop"] = "[システム] プロセス停止を要求しました...\n";
    m_dictJa["proc_send_sigkill"] = "[システム] 独立プロセス (PID: %1) をSIGKILLで強制終了します...\n";
    m_dictJa["proc_force_killed"] = "[システム] プロセスを強制終了(SIGKILL)しました。\n";
    m_dictJa["proc_finished"] = "[システム] プロセスが終了しました (終了コード: %1)\n";
    m_dictJa["proc_error_start"] = "[エラー] プロセスの起動に失敗しました。Pythonのパスまたはスクリプトパスを確認してください。\n";
    m_dictJa["proc_running"] = "[システム] 起動完了 (PID: %1)\n";


    // --- English Dictionary ---
    m_dictEn["app_window_title"] = "Python App Launcher";
    m_dictEn["app_title"] = "Python App Launcher";
    m_dictEn["app_subtitle"] = "No terminal required • Safe, one-click Python script execution";
    m_dictEn["search_placeholder"] = "🔍 Search by app name...";
    m_dictEn["btn_add_sample"] = "Add Sample Apps";
    m_dictEn["btn_add_app"] = "＋ Add New App";
    m_dictEn["hint_drag_drop"] = "💡 Tip: Drag and drop .py files directly into this window to register them.";
    m_dictEn["lang_select"] = "🌐 Language / 言語";

    // Desktop Shortcut Button
    m_dictEn["btn_desktop_register"] = "🖥️ Add to System Menu";
    m_dictEn["tip_desktop_register"] = "Add this launcher to Ubuntu's Application Menu";
    m_dictEn["btn_desktop_unregister"] = "🗑️ Remove from Menu";
    m_dictEn["tip_desktop_unregister"] = "Remove this launcher from Ubuntu's Application Menu";

    // Desktop Shortcut Messages
    m_dictEn["msg_desktop_reg_confirm_title"] = "Confirm Removal";
    m_dictEn["msg_desktop_reg_confirm_text"] = "Remove 'Python Launcher' from the Ubuntu application menu?";
    m_dictEn["msg_desktop_unreg_success_title"] = "Removed";
    m_dictEn["msg_desktop_unreg_success_text"] = "Successfully removed from the application menu.";
    m_dictEn["msg_desktop_unreg_failed"] = "Failed to remove desktop entry file.";
    m_dictEn["msg_desktop_reg_success_title"] = "Registered";
    m_dictEn["msg_desktop_reg_success_text"] = "'Python Launcher' has been added to your application menu!\n\n【How to Launch】\n• Search 'Python Launcher' in Show Applications or press the Super key.";
    m_dictEn["msg_desktop_error_title"] = "Error";
    m_dictEn["msg_desktop_dir_error"] = "Could not resolve the applications directory.";
    m_dictEn["msg_desktop_create_error"] = "Failed to create desktop entry file:\n%1";

    // Card Items
    m_dictEn["card_badge_detached"] = "🔄 Detached";
    m_dictEn["card_tip_detached"] = "Runs continuously in background even if the launcher is closed";
    m_dictEn["card_status_running"] = "🟢 Running (PID: %1)";
    m_dictEn["card_status_starting"] = "🟡 Starting...";
    m_dictEn["card_status_failed"] = "🔴 Failed";
    m_dictEn["card_status_stopped"] = "⚪ Stopped";
    m_dictEn["card_no_desc"] = "(No description)";
    m_dictEn["card_btn_run"] = "▶ Run";
    m_dictEn["card_btn_stop"] = "■ Stop";
    m_dictEn["card_btn_log"] = "📄 Log";
    m_dictEn["card_tip_edit"] = "Edit";
    m_dictEn["card_tip_delete"] = "Delete";

    // Delete Confirmation
    m_dictEn["msg_delete_confirm_title"] = "Delete App";
    m_dictEn["msg_delete_confirm_text"] = "Are you sure you want to remove '%1' from the list?\n(The script file itself will not be deleted)";

    // Samples
    m_dictEn["sample_added_title"] = "Sample Apps Added";
    m_dictEn["sample_added_text"] = "Added 2 sample Python apps for testing!";
    m_dictEn["sample_gui_name"] = "Sample GUI App (Tkinter)";
    m_dictEn["sample_gui_desc"] = "Sample Tkinter GUI window with interactive message box";
    m_dictEn["sample_cli_name"] = "Sample CLI Tool (Realtime Output)";
    m_dictEn["sample_cli_desc"] = "Sample CLI script outputting realtime progress and errors";

    // Log Viewer
    m_dictEn["log_title"] = "📄 Execution Log Monitor (Terminal Output)";
    m_dictEn["log_target"] = "Target:";
    m_dictEn["log_all"] = "All Logs (Global)";
    m_dictEn["log_autoscroll"] = "Auto Scroll";
    m_dictEn["log_clear"] = "Clear Log";
    m_dictEn["log_save"] = "Save Log...";
    m_dictEn["log_save_dialog_title"] = "Save Log File";
    m_dictEn["log_save_filter"] = "Log Files (*.log *.txt);;All Files (*)";
    m_dictEn["log_save_error_title"] = "Save Failed";
    m_dictEn["log_save_error_text"] = "Failed to write log file.";
    m_dictEn["log_save_success_title"] = "Saved";
    m_dictEn["log_save_success_text"] = "Log saved successfully: %1";

    // Add / Edit App Dialog
    m_dictEn["dlg_title_add"] = "Register New Python App";
    m_dictEn["dlg_title_edit"] = "Edit Python App Settings";
    m_dictEn["dlg_header_add"] = "Register a new Python application";
    m_dictEn["dlg_header_edit"] = "Edit registered Python application details";
    m_dictEn["dlg_lbl_name"] = "App Name (*):";
    m_dictEn["dlg_ph_name"] = "e.g., Image Batch Resizer";
    m_dictEn["dlg_lbl_desc"] = "Description:";
    m_dictEn["dlg_ph_desc"] = "e.g., Automatically processes and resizes dropped images";
    m_dictEn["dlg_lbl_script"] = "Script Path (*):";
    m_dictEn["dlg_ph_script"] = "e.g., /home/user/scripts/main.py";
    m_dictEn["dlg_btn_browse"] = "Browse...";
    m_dictEn["dlg_lbl_interp"] = "Python Environment (Interp):";
    m_dictEn["dlg_ph_interp"] = "e.g., /usr/bin/python3 or venv/bin/python";
    m_dictEn["dlg_btn_autodetect"] = "Auto Detect";
    m_dictEn["dlg_lbl_workdir"] = "Working Directory:";
    m_dictEn["dlg_ph_workdir"] = "Defaults to script directory if empty";
    m_dictEn["dlg_lbl_args"] = "Arguments (Optional):";
    m_dictEn["dlg_ph_args"] = "e.g., --config settings.json --verbose";
    m_dictEn["dlg_lbl_icon"] = "App Icon:";
    m_dictEn["dlg_ph_icon"] = "Optional (.png, .jpg, .ico, .svg)";
    m_dictEn["dlg_lbl_mode"] = "Launch Mode:";
    m_dictEn["dlg_chk_keepalive"] = "Keep running in background after launcher closes (Detached)";
    m_dictEn["dlg_btn_cancel"] = "Cancel";
    m_dictEn["dlg_btn_save"] = "Save";
    m_dictEn["dlg_btn_register"] = "Register";

    // Dialog File Browse titles
    m_dictEn["dlg_browse_script_title"] = "Select Python Script";
    m_dictEn["dlg_browse_script_filter"] = "Python Files (*.py *.pyw);;All Files (*)";
    m_dictEn["dlg_browse_interp_title"] = "Select Python Executable";
    m_dictEn["dlg_browse_interp_filter"] = "Executable (python python3);;All Files (*)";
    m_dictEn["dlg_browse_workdir_title"] = "Select Working Directory";
    m_dictEn["dlg_browse_icon_title"] = "Select Icon Image";
    m_dictEn["dlg_browse_icon_filter"] = "Images (*.png *.jpg *.jpeg *.ico *.svg);;All Files (*)";

    // Dialog Validation
    m_dictEn["dlg_val_error_title"] = "Input Error";
    m_dictEn["dlg_val_name_empty"] = "Please enter an application name.";
    m_dictEn["dlg_val_script_empty"] = "Please specify the Python script path.";

    // Process Runner Messages
    m_dictEn["proc_detached_start"] = "[System] Starting detached process: %1 %2\n";
    m_dictEn["proc_detached_log_dest"] = "[System] Log output destination: %1\n";
    m_dictEn["proc_detached_bg_notice"] = "[System] Python will continue running in background even after closing the launcher.\n";
    m_dictEn["proc_detached_success"] = "[System] Detached process started successfully (PID: %1)\n";
    m_dictEn["proc_detached_failed"] = "[Error] Failed to start detached process.\n";
    m_dictEn["proc_normal_start"] = "[System] Starting: %1 %2\n";
    m_dictEn["proc_send_sigterm"] = "[System] Sending SIGTERM to detached process (PID: %1)...\n";
    m_dictEn["proc_req_stop"] = "[System] Process termination requested...\n";
    m_dictEn["proc_send_sigkill"] = "[System] Force killing (SIGKILL) detached process (PID: %1)...\n";
    m_dictEn["proc_force_killed"] = "[System] Process killed (SIGKILL).\n";
    m_dictEn["proc_finished"] = "[System] Process finished (exit code: %1)\n";
    m_dictEn["proc_error_start"] = "[Error] Failed to start process. Please check Python path or script path.\n";
    m_dictEn["proc_running"] = "[System] Started successfully (PID: %1)\n";
}
