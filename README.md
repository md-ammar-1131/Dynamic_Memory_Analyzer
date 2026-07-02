# Dynamic Memory Allocation Analysis Tool (Intel PIN)

A dynamic binary instrumentation tool built using **Intel PIN** with a modern **Python (CustomTkinter)** GUI for runtime memory analysis.

The tool acts like a **CCTV for heap memory**, monitoring memory allocation and deallocation events while a program executes—**without requiring the application's source code**. It intercepts memory management functions such as `malloc`, `calloc`, `realloc`, and `free` to detect memory leaks, invalid frees, double frees, and visualize heap behavior in real time.

---

## 🚀 Features

- 🔍 **Runtime Heap Monitoring**
  - Tracks dynamic memory allocations and deallocations while the program is running.

- ⚠️ **Memory Anomaly Detection**
  - Detects:
    - Memory leaks
    - Double free errors
    - Invalid pointer deallocations

- 🧵 **Thread-Aware Analysis**
  - Supports monitoring of multi-threaded C/C++ applications.

- 📊 **Interactive GUI Dashboard**
  - Built using **CustomTkinter** and **Matplotlib** for intuitive visualization.

- 📈 **Memory Allocation Visualization**
  - Displays heap allocation trajectories and runtime statistics.

- 🔧 **Binary Instrumentation**
  - Works directly on compiled executables using Intel PIN without modifying source code.

---

# 🛠️ Prerequisites

Before running the project, ensure the following are installed:

- Linux (or Windows using **WSL**)
- Intel PIN Kit
- GCC Compiler
- Python 3.x

### Required Python Packages

```bash
pip install customtkinter matplotlib numpy
```

---

# ⚙️ Installation & Setup

## 1. Configure Intel PIN Directory

Assuming the Intel PIN kit is located at:

```text
~/pin_kit_main
```

Navigate to the PIN tools directory and create a folder for the project.

```bash
cd ~/pin_kit_main/source/tools/
mkdir mypintool
cd mypintool
```

---

## 2. Add the Instrumentation Source

Copy the backend PIN tool source into the newly created directory.

```bash
cp /path/to/your/project/mypintool.cpp .
```

---

## 3. Compile the PIN Tool

Compile the instrumentation tool.

```bash
make
```

After compilation, a shared object similar to:

```text
obj-intel64/MyPinTool.so
```

will be generated.

---

## 4. Set Environment Variable

Export the Intel PIN installation directory.

```bash
export PIN_ROOT=~/pin_kit_main
```

---

# 💻 Usage

## Step 1: Compile Test Programs

Compile any C/C++ program you want to analyze.

Example:

```bash
gcc -pthread test1.c -o test1
gcc -pthread test2.c -o test2
```

You may also analyze your own executables.

---

## Step 2: Generate Memory Trace

Run the executable through Intel PIN.

```bash
$PIN_ROOT/pin -t obj-intel64/MyPinTool.so -- ./test1
```

or

```bash
$PIN_ROOT/pin -t obj-intel64/MyPinTool.so -- ./test2
```

This generates a memory trace file (for example):

```text
memtrace.out
```

---

## Step 3: Launch the Analyzer GUI

Start the Python dashboard.

```bash
python gui.py
```

Inside the application:

1. Click **Load Trace File**
2. Select the generated **memtrace.out**
3. Explore:
   - Memory allocations
   - Memory deallocations
   - Heap usage
   - Thread activity
   - Leak detection
   - Double free detection
   - Invalid free detection

---

# 📂 Project Workflow

```text
C/C++ Program
      │
      ▼
 Intel PIN Tool
      │
      ▼
Memory Trace (memtrace.out)
      │
      ▼
Python GUI Analyzer
      │
      ▼
Visualization + Memory Analysis
```

---

# 🧰 Technologies Used

- C++
- Intel PIN
- Python
- CustomTkinter
- Matplotlib
- NumPy
- GCC
- Linux / WSL

---

# 📌 Example Commands

Compile:

```bash
gcc -pthread test1.c -o test1
```

Run through PIN:

```bash
$PIN_ROOT/pin -t obj-intel64/MyPinTool.so -- ./test1
```

Launch GUI:

```bash
python gui.py
```

---

# 👥 Authors

- **Md Ammar**
- **Aman Raj**

Developed as part of the **CS204** course project.
