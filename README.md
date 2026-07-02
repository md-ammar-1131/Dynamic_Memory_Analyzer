# Dynamic Memory Allocation Analysis Tool (Intel PIN)

This project is a dynamic binary instrumentation tool built with Intel PIN and a modern Python (CustomTkinter) GUI. [cite_start]It acts as a "CCTV" for your system's memory [cite: 41, 42][cite_start], analyzing memory usage while a program is running without needing its source code[cite: 40]. 

[cite_start]It tracks `malloc`, `calloc`, `realloc`, and `free` operations to detect memory leaks, monitor multi-threaded activity, and visualize heap allocation trajectories in real-time[cite: 8, 10].

## 🚀 Features
* [cite_start]**Runtime Monitoring:** Track heap allocations and deallocations on the fly[cite: 8].
* [cite_start]**Anomaly Detection:** Automatically identifies memory leaks, double frees, and invalid pointer deallocations[cite: 10].
* [cite_start]**Thread-Aware:** Full support for monitoring multi-threaded C/C++ applications[cite: 10].
* **Modern GUI:** Interactive dashboards built with CustomTkinter and Matplotlib for data visualization.

---

## 🛠️ Prerequisites
Before running the tool, ensure you have the following installed:
* **Linux Environment** (WSL - Windows Subsystem for Linux is fully supported)
* **Intel PIN Kit**
* **GCC Compiler** (for compiling test programs)
* **Python 3.x**
* **Python Packages:** `customtkinter`, `matplotlib`, `numpy`
  ```bash
  pip install customtkinter matplotlib numpy
