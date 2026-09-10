# Python App Launcher (Qt C++)

[English](README.md) | [日本語](README.jp.md)

---

A **GUI-based Python application launcher and manager** designed for users on Linux/Ubuntu who want to run, manage, and monitor Python scripts effortlessly without using the terminal.

---

## 🌟 Key Features

1. **Multi-Language Support (English / Japanese Switching)**:
   - Easily switch between English and Japanese in real-time from the header dropdown. Selected language is saved automatically.
2. **One-Click Ubuntu Application Menu Integration**:
   - Click the **"🖥️ Add to System Menu"** button next to the title to add Python Launcher to Ubuntu's Application Menu (Super key search).
   - If already registered, it switches to **"🗑️ Remove from Menu"** to remove the shortcut anytime with a single click.
3. **Pure GUI Operations (No Terminal Needed)**:
   - Register, edit, run, and stop Python scripts (`.py`, `.pyw`) with simple button clicks.
   - Supports **Drag & Drop** of `.py` files directly into the window for quick registration.
4. **Automatic Python & Virtual Environment (venv) Detection**:
   - Automatically detects `venv` and `.venv` environments inside the script directory or its parent directory.
   - Switch between system Python (`/usr/bin/python3`) and custom virtual environments with ease.
5. **Detached Launch (Background Execution)**:
   - Supports detached background execution so GUI tools and services keep running even after you close the Python Launcher.
6. **Real-Time GUI Log Monitor**:
   - View `stdout` and `stderr` in real-time with color coding without opening a terminal.
   - Filter logs by application, auto-scroll, clear logs, and export logs to file.
7. **Automatic Configuration Persistence**:
   - Registered applications and settings are automatically saved in JSON format and restored on the next startup.

---

## 🚀 Getting Started

### 1. Launching the App
Double-click `run.sh` or run `./run.sh` in the terminal to start the launcher.

### 2. Registering to Ubuntu Application Menu
After launching, click **"🖥️ Add to System Menu"** in the top header. "Python Launcher" will be available in Ubuntu's "Show Applications" menu (Super key search).

---

## 🛠️ Build Instructions

### Prerequisites (Ubuntu / Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake qtbase5-dev
```

### Build & Run
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
./PythonLauncher
```

---

## 🧪 Included Sample Applications

Sample scripts are included for quick testing:
- **Sample GUI App (Tkinter)**: Tests whether GUI windows display and function properly.
- **Sample CLI Tool**: Outputs real-time progress and logs to the built-in log viewer.

---

## 📄 License
MIT License
