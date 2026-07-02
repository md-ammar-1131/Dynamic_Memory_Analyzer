
import tkinter as tk
from tkinter import ttk, filedialog
import re
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

class MemoryAnalyzerV3:
    def __init__(self, root):
        self.root = root
        self.root.title("Intel PIN Memory Trace Analyzer - V3 (TLS)")
        self.root.geometry("1100x750")

        self.top_frame = ttk.Frame(root, padding=10)
        self.top_frame.pack(side=tk.TOP, fill=tk.X)

        self.load_btn = ttk.Button(self.top_frame, text="Load trace.out", command=self.load_file)
        self.load_btn.pack(side=tk.LEFT)
       
        self.status_var = tk.StringVar(value="No file loaded.")
        self.status_lbl = ttk.Label(self.top_frame, textvariable=self.status_var)
        self.status_lbl.pack(side=tk.LEFT, padx=10)

        self.notebook = ttk.Notebook(root)
        self.notebook.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)

        self.tab_dashboard = ttk.Frame(self.notebook)
        self.tab_thread_stats = ttk.Frame(self.notebook)
        self.tab_anomalies = ttk.Frame(self.notebook)
        self.tab_report = ttk.Frame(self.notebook)

        self.notebook.add(self.tab_dashboard, text="Global Memory Graph")
        self.notebook.add(self.tab_thread_stats, text="Per-Thread Activity")
        self.notebook.add(self.tab_anomalies, text="Anomalies & Leaks")
        self.notebook.add(self.tab_report, text="Raw Final Report")

        # Global Memory Graph Setup
        self.fig_mem = Figure(figsize=(8, 4), dpi=100)
        self.ax_mem = self.fig_mem.add_subplot(111)
        self.canvas_mem = FigureCanvasTkAgg(self.fig_mem, master=self.tab_dashboard)
        self.canvas_mem.get_tk_widget().pack(expand=True, fill=tk.BOTH)

        # Thread Activity Graph Setup
        self.fig_threads = Figure(figsize=(8, 4), dpi=100)
        self.ax_threads = self.fig_threads.add_subplot(111)
        self.canvas_threads = FigureCanvasTkAgg(self.fig_threads, master=self.tab_thread_stats)
        self.canvas_threads.get_tk_widget().pack(expand=True, fill=tk.BOTH)

        # Text Areas
        self.anomaly_text = tk.Text(self.tab_anomalies, wrap="word", font=("Consolas", 10))
        self.anomaly_text.pack(expand=True, fill=tk.BOTH)
       
        self.report_text = tk.Text(self.tab_report, wrap="word", font=("Consolas", 10))
        self.report_text.pack(expand=True, fill=tk.BOTH)

    def load_file(self):
        filepath = filedialog.askopenfilename(filetypes=[("Text Files", "*.out"), ("All Files", "*.*")])
        if filepath:
            self.status_var.set(f"Loaded: {filepath}")
            self.parse_data(filepath)

    def parse_data(self, filepath):
        time_steps, memory_usage = [], []
        current_mem, step = 0, 0
        anomalies, report_lines = [], []
       
        thread_data = {} # {thread_id: {'allocs': 0, 'frees': 0}}
        current_thread_parsing = None

        with open(filepath, 'r') as file:
            for line in file:
                # 1. Parse Time Series & Anomalies
                if "[Thread" in line:
                    alloc_match = re.search(r'ALLOC .*?Size=(\d+)', line)
                    free_match = re.search(r'FREE .*?Size=(\d+)', line)
                    if alloc_match:
                        current_mem += int(alloc_match.group(1))
                        step += 1
                        time_steps.append(step)
                        memory_usage.append(current_mem)
                    elif free_match:
                        current_mem -= int(free_match.group(1))
                        step += 1
                        time_steps.append(step)
                        memory_usage.append(current_mem)
                       
                if "[ERROR]" in line or "-> Leaked Address:" in line:
                    anomalies.append(line.strip())

                # 2. Parse Final Report and Thread Data
                report_lines.append(line)
               
                thread_match = re.search(r'\[THREAD (\d+) SUMMARY\]', line)
                if thread_match:
                    current_thread_parsing = thread_match.group(1)
                    thread_data[current_thread_parsing] = {}
                    continue
               
                if current_thread_parsing:
                    if "Allocs:" in line:
                        thread_data[current_thread_parsing]['allocs'] = int(re.search(r'Allocs:\s+(\d+)', line).group(1))
                    elif "Frees :" in line:
                        thread_data[current_thread_parsing]['frees'] = int(re.search(r'Frees :\s+(\d+)', line).group(1))
                        current_thread_parsing = None # End of block

        self.update_gui(time_steps, memory_usage, thread_data, anomalies, report_lines)

    def update_gui(self, time_steps, memory_usage, thread_data, anomalies, report_lines):
        # 1. Global Memory Chart
        self.ax_mem.clear()
        self.ax_mem.plot(time_steps, memory_usage, color='purple')
        self.ax_mem.fill_between(time_steps, memory_usage, color='purple', alpha=0.3)
        self.ax_mem.set_title("Global Memory State")
        self.ax_mem.set_ylabel("Bytes Allocated")
        self.ax_mem.grid(True)
        self.canvas_mem.draw()

        # 2. Thread Activity Bar Chart
        self.ax_threads.clear()
        if thread_data:
            threads = list(thread_data.keys())
            allocs = [thread_data[t].get('allocs', 0) for t in threads]
            frees = [thread_data[t].get('frees', 0) for t in threads]
           
            x = np.arange(len(threads))
            width = 0.35
           
            self.ax_threads.bar(x - width/2, allocs, width, label='Allocations', color='green')
            self.ax_threads.bar(x + width/2, frees, width, label='Frees', color='red')
           
            self.ax_threads.set_xlabel('Thread ID')
            self.ax_threads.set_ylabel('Function Calls')
            self.ax_threads.set_title('Per-Thread Workload Distribution')
            self.ax_threads.set_xticks(x)
            self.ax_threads.set_xticklabels([f"T-{t}" for t in threads])
            self.ax_threads.legend()
        else:
            self.ax_threads.text(0.5, 0.5, "No Thread Summaries Found in Log", ha='center', va='center')
        self.canvas_threads.draw()

        # 3. Text Fields
        self.anomaly_text.delete(1.0, tk.END)
        self.anomaly_text.insert(tk.END, "\n".join(anomalies) if anomalies else "Clean! No memory leaks or invalid frees.")
       
        self.report_text.delete(1.0, tk.END)
        self.report_text.insert(tk.END, "".join(report_lines))

if __name__ == "__main__":
    root = tk.Tk()
    app = MemoryAnalyzerV3(root)
    root.mainloop()