
# import tkinter as tk
# from tkinter import ttk, filedialog
# import re
# from matplotlib.figure import Figure
# from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
# import numpy as np

# class MemoryAnalyzerV3:
#     def __init__(self, root):
#         self.root = root
#         self.root.title("Intel PIN Memory Trace Analyzer - V3 (TLS)")
#         self.root.geometry("1100x750")

#         self.top_frame = ttk.Frame(root, padding=10)
#         self.top_frame.pack(side=tk.TOP, fill=tk.X)

#         self.load_btn = ttk.Button(self.top_frame, text="Load trace.out", command=self.load_file)
#         self.load_btn.pack(side=tk.LEFT)
       
#         self.status_var = tk.StringVar(value="No file loaded.")
#         self.status_lbl = ttk.Label(self.top_frame, textvariable=self.status_var)
#         self.status_lbl.pack(side=tk.LEFT, padx=10)

#         self.notebook = ttk.Notebook(root)
#         self.notebook.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)

#         self.tab_dashboard = ttk.Frame(self.notebook)
#         self.tab_thread_stats = ttk.Frame(self.notebook)
#         self.tab_anomalies = ttk.Frame(self.notebook)
#         self.tab_report = ttk.Frame(self.notebook)

#         self.notebook.add(self.tab_dashboard, text="Global Memory Graph")
#         self.notebook.add(self.tab_thread_stats, text="Per-Thread Activity")
#         self.notebook.add(self.tab_anomalies, text="Anomalies & Leaks")
#         self.notebook.add(self.tab_report, text="Raw Final Report")

#         # Global Memory Graph Setup
#         self.fig_mem = Figure(figsize=(8, 4), dpi=100)
#         self.ax_mem = self.fig_mem.add_subplot(111)
#         self.canvas_mem = FigureCanvasTkAgg(self.fig_mem, master=self.tab_dashboard)
#         self.canvas_mem.get_tk_widget().pack(expand=True, fill=tk.BOTH)

#         # Thread Activity Graph Setup
#         self.fig_threads = Figure(figsize=(8, 4), dpi=100)
#         self.ax_threads = self.fig_threads.add_subplot(111)
#         self.canvas_threads = FigureCanvasTkAgg(self.fig_threads, master=self.tab_thread_stats)
#         self.canvas_threads.get_tk_widget().pack(expand=True, fill=tk.BOTH)

#         # Text Areas
#         self.anomaly_text = tk.Text(self.tab_anomalies, wrap="word", font=("Consolas", 10))
#         self.anomaly_text.pack(expand=True, fill=tk.BOTH)
       
#         self.report_text = tk.Text(self.tab_report, wrap="word", font=("Consolas", 10))
#         self.report_text.pack(expand=True, fill=tk.BOTH)

#     def load_file(self):
#         filepath = filedialog.askopenfilename(filetypes=[("Text Files", "*.out"), ("All Files", "*.*")])
#         if filepath:
#             self.status_var.set(f"Loaded: {filepath}")
#             self.parse_data(filepath)

#     def parse_data(self, filepath):
#         time_steps, memory_usage = [], []
#         current_mem, step = 0, 0
#         anomalies, report_lines = [], []
       
#         thread_data = {} # {thread_id: {'allocs': 0, 'frees': 0}}
#         current_thread_parsing = None

#         with open(filepath, 'r') as file:
#             for line in file:
#                 # 1. Parse Time Series & Anomalies
#                 if "[Thread" in line:
#                     alloc_match = re.search(r'ALLOC .*?Size=(\d+)', line)
#                     free_match = re.search(r'FREE .*?Size=(\d+)', line)
#                     if alloc_match:
#                         current_mem += int(alloc_match.group(1))
#                         step += 1
#                         time_steps.append(step)
#                         memory_usage.append(current_mem)
#                     elif free_match:
#                         current_mem -= int(free_match.group(1))
#                         step += 1
#                         time_steps.append(step)
#                         memory_usage.append(current_mem)
                       
#                 if "[ERROR]" in line or "-> Leaked Address:" in line:
#                     anomalies.append(line.strip())

#                 # 2. Parse Final Report and Thread Data
#                 report_lines.append(line)
               
#                 thread_match = re.search(r'\[THREAD (\d+) SUMMARY\]', line)
#                 if thread_match:
#                     current_thread_parsing = thread_match.group(1)
#                     thread_data[current_thread_parsing] = {}
#                     continue
               
#                 if current_thread_parsing:
#                     if "Allocs:" in line:
#                         thread_data[current_thread_parsing]['allocs'] = int(re.search(r'Allocs:\s+(\d+)', line).group(1))
#                     elif "Frees :" in line:
#                         thread_data[current_thread_parsing]['frees'] = int(re.search(r'Frees :\s+(\d+)', line).group(1))
#                         current_thread_parsing = None # End of block

#         self.update_gui(time_steps, memory_usage, thread_data, anomalies, report_lines)

#     def update_gui(self, time_steps, memory_usage, thread_data, anomalies, report_lines):
#         # 1. Global Memory Chart
#         self.ax_mem.clear()
#         self.ax_mem.plot(time_steps, memory_usage, color='purple')
#         self.ax_mem.fill_between(time_steps, memory_usage, color='purple', alpha=0.3)
#         self.ax_mem.set_title("Global Memory State")
#         self.ax_mem.set_ylabel("Bytes Allocated")
#         self.ax_mem.grid(True)
#         self.canvas_mem.draw()

#         # 2. Thread Activity Bar Chart
#         self.ax_threads.clear()
#         if thread_data:
#             threads = list(thread_data.keys())
#             allocs = [thread_data[t].get('allocs', 0) for t in threads]
#             frees = [thread_data[t].get('frees', 0) for t in threads]
           
#             x = np.arange(len(threads))
#             width = 0.35
           
#             self.ax_threads.bar(x - width/2, allocs, width, label='Allocations', color='green')
#             self.ax_threads.bar(x + width/2, frees, width, label='Frees', color='red')
           
#             self.ax_threads.set_xlabel('Thread ID')
#             self.ax_threads.set_ylabel('Function Calls')
#             self.ax_threads.set_title('Per-Thread Workload Distribution')
#             self.ax_threads.set_xticks(x)
#             self.ax_threads.set_xticklabels([f"T-{t}" for t in threads])
#             self.ax_threads.legend()
#         else:
#             self.ax_threads.text(0.5, 0.5, "No Thread Summaries Found in Log", ha='center', va='center')
#         self.canvas_threads.draw()

#         # 3. Text Fields
#         self.anomaly_text.delete(1.0, tk.END)
#         self.anomaly_text.insert(tk.END, "\n".join(anomalies) if anomalies else "Clean! No memory leaks or invalid frees.")
       
#         self.report_text.delete(1.0, tk.END)
#         self.report_text.insert(tk.END, "".join(report_lines))

# if __name__ == "__main__":
#     root = tk.Tk()
#     app = MemoryAnalyzerV3(root)
#     root.mainloop()

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import re
import csv
import os
from typing import Dict, List, Any
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

# Apply dark theme to matplotlib
plt.style.use('dark_background')

class MemoryTraceParser:
    """Handles the parsing and data extraction from Intel PIN trace logs."""
    
    @staticmethod
    def parse(filepath: str) -> Dict[str, Any]:
        time_steps = []
        memory_usage = []
        alloc_sizes = []
        events = []
        
        leaks = []
        invalid_frees = []
        report_lines = []
        
        thread_data = {}
        thread_memory_usage = {}
        
        current_mem = 0
        step = 0
        current_thread_parsing = None

        try:
            with open(filepath, 'r') as file:
                for line in file:
                    line = line.strip()
                    if not line:
                        continue
                        
                    # Parse Thread ID if present
                    thread_id = "0"
                    t_match = re.search(r'\[Thread\s*(\d+)\]', line, re.IGNORECASE)
                    if t_match:
                        thread_id = t_match.group(1)
                        
                    if thread_id not in thread_data:
                        thread_data[thread_id] = {'allocs': 0, 'frees': 0, 'current_mem': 0}
                        thread_memory_usage[thread_id] = {'steps': [], 'mem': []}

                    # Memory Operations
                    alloc_match = re.search(r'ALLOC .*?Size=(\d+)', line)
                    free_match = re.search(r'FREE .*?Size=(\d+)', line)
                    
                    if alloc_match:
                        size = int(alloc_match.group(1))
                        current_mem += size
                        thread_data[thread_id]['allocs'] += 1
                        thread_data[thread_id]['current_mem'] += size
                        alloc_sizes.append(size)
                        
                        step += 1
                        time_steps.append(step)
                        memory_usage.append(current_mem)
                        
                        thread_memory_usage[thread_id]['steps'].append(step)
                        thread_memory_usage[thread_id]['mem'].append(thread_data[thread_id]['current_mem'])
                        
                        events.append((step, thread_id, "ALLOC", size, current_mem, line))
                        
                    elif free_match:
                        size = int(free_match.group(1))
                        current_mem = max(0, current_mem - size)
                        thread_data[thread_id]['frees'] += 1
                        thread_data[thread_id]['current_mem'] = max(0, thread_data[thread_id]['current_mem'] - size)
                        
                        step += 1
                        time_steps.append(step)
                        memory_usage.append(current_mem)
                        
                        thread_memory_usage[thread_id]['steps'].append(step)
                        thread_memory_usage[thread_id]['mem'].append(thread_data[thread_id]['current_mem'])
                        
                        events.append((step, thread_id, "FREE", size, current_mem, line))
                        
                    # Anomalies
                    if "Leaked" in line or "Leak:" in line:
                        leaks.append(line)
                    elif "Invalid Free" in line or "Double Free" in line or "[ERROR]" in line:
                        invalid_frees.append(line)

                    # Summary block parsing
                    report_lines.append(line)
                    summary_match = re.search(r'\[THREAD (\d+) SUMMARY\]', line)
                    if summary_match:
                        current_thread_parsing = summary_match.group(1)
                        if current_thread_parsing not in thread_data:
                            thread_data[current_thread_parsing] = {'allocs': 0, 'frees': 0}
                        continue
                    
                    if current_thread_parsing:
                        a_match = re.search(r'Allocs:\s+(\d+)', line)
                        f_match = re.search(r'Frees :\s+(\d+)', line)
                        if a_match:
                            thread_data[current_thread_parsing]['allocs'] = int(a_match.group(1))
                        elif f_match:
                            thread_data[current_thread_parsing]['frees'] = int(f_match.group(1))
                            current_thread_parsing = None
                            
            return {
                "time_steps": time_steps,
                "memory_usage": memory_usage,
                "alloc_sizes": alloc_sizes,
                "events": events,
                "thread_data": thread_data,
                "thread_memory_usage": thread_memory_usage,
                "leaks": leaks,
                "invalid_frees": invalid_frees,
                "report_lines": report_lines
            }
            
        except Exception as e:
            raise RuntimeError(f"Failed to parse trace file: {str(e)}")


class MemoryAnalyzerGUI:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("Dynamic Memory Analysis Tool | Profiler")
        self.root.geometry("1400x850")
        
        self.data_cache = None
        self._setup_dark_theme()
        self._build_ui()

    def _setup_dark_theme(self):
        """Configures a professional dark theme across Tkinter components."""
        self.bg_main = "#1e1e1e"
        self.bg_panel = "#252526"
        self.bg_accent = "#333333"
        self.fg_main = "#cccccc"
        self.fg_accent = "#4da6ff"
        self.color_alloc = "#4caf50"
        self.color_free = "#f44336"

        style = ttk.Style()
        style.theme_use('clam')
        
        style.configure(".", background=self.bg_main, foreground=self.fg_main, font=("Segoe UI", 10))
        style.configure("TFrame", background=self.bg_main)
        style.configure("Panel.TFrame", background=self.bg_panel)
        
        style.configure("TLabel", background=self.bg_main, foreground=self.fg_main)
        style.configure("Header.TLabel", font=("Segoe UI", 16, "bold"), foreground=self.fg_accent)
        style.configure("SubHeader.TLabel", font=("Segoe UI", 12, "bold"))
        
        style.configure("TButton", background=self.bg_accent, foreground=self.fg_main, padding=6, borderwidth=0)
        style.map("TButton", background=[("active", "#404040")])
        
        style.configure("TNotebook", background=self.bg_main, borderwidth=0)
        style.configure("TNotebook.Tab", background=self.bg_panel, foreground=self.fg_main, padding=[15, 5], font=("Segoe UI", 10, "bold"))
        style.map("TNotebook.Tab", background=[("selected", self.bg_accent)], foreground=[("selected", "#ffffff")])

        style.configure("Treeview", background=self.bg_panel, foreground=self.fg_main, fieldbackground=self.bg_panel, borderwidth=0)
        style.configure("Treeview.Heading", background=self.bg_accent, foreground=self.fg_main, font=("Segoe UI", 10, "bold"))
        style.map("Treeview", background=[("selected", "#094771")])

    def _build_ui(self):
        self.root.configure(bg=self.bg_main)

        # Top Control Bar
        self.top_frame = ttk.Frame(self.root, padding="15 15 15 15", style="Panel.TFrame")
        self.top_frame.pack(side=tk.TOP, fill=tk.X)

        self.load_btn = ttk.Button(self.top_frame, text="📁 Load Trace File", command=self.load_file)
        self.load_btn.pack(side=tk.LEFT)
        
        self.export_btn = ttk.Button(self.top_frame, text="💾 Export CSV", command=self.export_csv, state=tk.DISABLED)
        self.export_btn.pack(side=tk.LEFT, padx=10)

        self.status_var = tk.StringVar(value="Status: Ready")
        self.status_lbl = ttk.Label(self.top_frame, textvariable=self.status_var, style="SubHeader.TLabel", background=self.bg_panel)
        self.status_lbl.pack(side=tk.RIGHT)

        # Notebook
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)

        # Tabs
        self.tab_dash = ttk.Frame(self.notebook)
        self.tab_timeline = ttk.Frame(self.notebook)
        self.tab_threads = ttk.Frame(self.notebook)
        self.tab_hist = ttk.Frame(self.notebook)
        self.tab_events = ttk.Frame(self.notebook)
        self.tab_anomalies = ttk.Frame(self.notebook)
        self.tab_report = ttk.Frame(self.notebook)

        self.notebook.add(self.tab_dash, text="📊 Dashboard")
        self.notebook.add(self.tab_timeline, text="📈 Heap Timeline")
        self.notebook.add(self.tab_threads, text="🔄 Per-Thread Stats")
        self.notebook.add(self.tab_hist, text="📉 Alloc Histogram")
        self.notebook.add(self.tab_events, text="🔍 Searchable Log")
        self.notebook.add(self.tab_anomalies, text="⚠️ Anomalies")
        self.notebook.add(self.tab_report, text="📄 Raw Report")

        self._init_dashboard()
        self._init_graphs()
        self._init_event_log()
        self._init_text_areas()

    def _init_dashboard(self):
        self.dash_frame = ttk.Frame(self.tab_dash, padding=20)
        self.dash_frame.pack(expand=True, fill=tk.BOTH)
        
        ttk.Label(self.dash_frame, text="Execution Summary", style="Header.TLabel").pack(anchor=tk.W, pady=(0, 20))
        
        self.lbl_peak_mem = ttk.Label(self.dash_frame, text="Peak Memory: N/A", font=("Segoe UI", 12))
        self.lbl_peak_mem.pack(anchor=tk.W, pady=5)
        
        self.lbl_total_allocs = ttk.Label(self.dash_frame, text="Total Allocations: N/A", font=("Segoe UI", 12))
        self.lbl_total_allocs.pack(anchor=tk.W, pady=5)
        
        self.lbl_total_frees = ttk.Label(self.dash_frame, text="Total Frees: N/A", font=("Segoe UI", 12))
        self.lbl_total_frees.pack(anchor=tk.W, pady=5)
        
        self.lbl_active_threads = ttk.Label(self.dash_frame, text="Active Threads: N/A", font=("Segoe UI", 12))
        self.lbl_active_threads.pack(anchor=tk.W, pady=5)
        
        self.lbl_leaks = ttk.Label(self.dash_frame, text="Potential Leaks: N/A", font=("Segoe UI", 12, "bold"))
        self.lbl_leaks.pack(anchor=tk.W, pady=5)
        
        self.lbl_inv_frees = ttk.Label(self.dash_frame, text="Invalid Frees: N/A", font=("Segoe UI", 12, "bold"))
        self.lbl_inv_frees.pack(anchor=tk.W, pady=5)

    def _init_graphs(self):
        # Setup Figure generator
        def make_canvas(parent):
            fig = Figure(figsize=(8, 4), dpi=100, facecolor=self.bg_main)
            ax = fig.add_subplot(111)
            ax.set_facecolor(self.bg_panel)
            ax.tick_params(colors=self.fg_main)
            for spine in ax.spines.values():
                spine.set_color('#555555')
            canvas = FigureCanvasTkAgg(fig, master=parent)
            canvas.get_tk_widget().pack(expand=True, fill=tk.BOTH, padx=10, pady=10)
            return fig, ax, canvas

        self.fig_mem, self.ax_mem, self.canvas_mem = make_canvas(self.tab_timeline)
        self.fig_threads, self.ax_threads, self.canvas_threads = make_canvas(self.tab_threads)
        self.fig_hist, self.ax_hist, self.canvas_hist = make_canvas(self.tab_hist)

    def _init_event_log(self):
        ctrl_frame = ttk.Frame(self.tab_events)
        ctrl_frame.pack(fill=tk.X, pady=5, padx=10)
        
        ttk.Label(ctrl_frame, text="Search:").pack(side=tk.LEFT)
        self.search_var = tk.StringVar()
        self.search_var.trace("w", self.filter_events)
        
        search_entry = tk.Entry(ctrl_frame, textvariable=self.search_var, bg=self.bg_panel, fg=self.fg_main, insertbackground=self.fg_main)
        search_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=10)
        
        # Treeview
        columns = ("step", "thread", "op", "size", "total_mem", "raw")
        self.tree = ttk.Treeview(self.tab_events, columns=columns, show="headings", selectmode="browse")
        
        self.tree.heading("step", text="Step")
        self.tree.heading("thread", text="Thread ID")
        self.tree.heading("op", text="Operation")
        self.tree.heading("size", text="Size (B)")
        self.tree.heading("total_mem", text="Total Mem (B)")
        self.tree.heading("raw", text="Raw Log")
        
        self.tree.column("step", width=80, anchor=tk.CENTER)
        self.tree.column("thread", width=80, anchor=tk.CENTER)
        self.tree.column("op", width=100, anchor=tk.CENTER)
        self.tree.column("size", width=100, anchor=tk.E)
        self.tree.column("total_mem", width=120, anchor=tk.E)
        self.tree.column("raw", width=500, anchor=tk.W)
        
        vsb = ttk.Scrollbar(self.tab_events, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        
        self.tree.pack(side=tk.LEFT, expand=True, fill=tk.BOTH, padx=(10,0), pady=10)
        vsb.pack(side=tk.RIGHT, fill=tk.Y, padx=(0,10), pady=10)

    def _init_text_areas(self):
        text_cfg = {"font": ("Consolas", 10), "bg": self.bg_panel, "fg": self.fg_main, "insertbackground": self.fg_main}
        
        # Anomalies
        self.anomaly_text = tk.Text(self.tab_anomalies, wrap="word", **text_cfg)
        self.anomaly_text.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)
        
        # Report
        self.report_text = tk.Text(self.tab_report, wrap="word", **text_cfg)
        self.report_text.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)

    def load_file(self):
        filepath = filedialog.askopenfilename(
            title="Select Trace File",
            filetypes=[("Log & Out Files", "*.out *.log"), ("All Files", "*.*")]
        )
        if not filepath:
            return
            
        self.status_var.set("Status: Parsing file (this may take a moment)...")
        self.root.update()
        
        try:
            self.data_cache = MemoryTraceParser.parse(filepath)
            self.update_gui()
            self.export_btn.config(state=tk.NORMAL)
            self.status_var.set(f"Status: Loaded {os.path.basename(filepath)}")
        except Exception as e:
            messagebox.showerror("Error", str(e))
            self.status_var.set("Status: Error loading file")

    def update_gui(self):
        if not self.data_cache: return
        d = self.data_cache
        
        # 1. Dashboard
        peak = max(d["memory_usage"]) if d["memory_usage"] else 0
        total_allocs = sum(t['allocs'] for t in d["thread_data"].values())
        total_frees = sum(t['frees'] for t in d["thread_data"].values())
        
        self.lbl_peak_mem.config(text=f"Peak Memory: {peak:,} Bytes")
        self.lbl_total_allocs.config(text=f"Total Allocations: {total_allocs:,}")
        self.lbl_total_frees.config(text=f"Total Frees: {total_frees:,}")
        self.lbl_active_threads.config(text=f"Active Threads: {len(d['thread_data'])}")
        
        self.lbl_leaks.config(text=f"Potential Leaks: {len(d['leaks'])}", foreground=self.color_free if d['leaks'] else self.color_alloc)
        self.lbl_inv_frees.config(text=f"Invalid Frees: {len(d['invalid_frees'])}", foreground=self.color_free if d['invalid_frees'] else self.color_alloc)

        # 2. Heap Timeline (Total vs Per-Thread)
        self.ax_mem.clear()
        self.ax_mem.plot(d["time_steps"], d["memory_usage"], color=self.fg_accent, linewidth=1.5, label='Total Heap')
        
        # Plot individual thread memory footprints if thread count is manageable
        if len(d["thread_memory_usage"]) <= 10:
            for tid, th_data in d["thread_memory_usage"].items():
                if th_data['steps']:
                    self.ax_mem.plot(th_data['steps'], th_data['mem'], alpha=0.5, linewidth=1, label=f'Thread {tid}')
                    
        self.ax_mem.set_title("Memory Allocation Timeline")
        self.ax_mem.set_ylabel("Bytes Allocated")
        self.ax_mem.legend()
        self.fig_mem.tight_layout()
        self.canvas_mem.draw()

        # 3. Thread Stats Bar Chart
        self.ax_threads.clear()
        if d["thread_data"]:
            threads = list(d["thread_data"].keys())
            allocs = [d["thread_data"][t]['allocs'] for t in threads]
            frees = [d["thread_data"][t]['frees'] for t in threads]
            x = np.arange(len(threads))
            width = 0.35
            
            self.ax_threads.bar(x - width/2, allocs, width, label='Allocations', color=self.color_alloc)
            self.ax_threads.bar(x + width/2, frees, width, label='Frees', color=self.color_free)
            self.ax_threads.set_xticks(x)
            self.ax_threads.set_xticklabels([f"T-{t}" for t in threads])
            self.ax_threads.set_title("Thread Operation Workload")
            self.ax_threads.legend()
        self.fig_threads.tight_layout()
        self.canvas_threads.draw()

        # 4. Allocation Histogram
        self.ax_hist.clear()
        if d["alloc_sizes"]:
            self.ax_hist.hist(d["alloc_sizes"], bins=50, color='#9b59b6', edgecolor='black')
            self.ax_hist.set_title("Allocation Size Distribution")
            self.ax_hist.set_xlabel("Size (Bytes)")
            self.ax_hist.set_ylabel("Frequency")
        self.fig_hist.tight_layout()
        self.canvas_hist.draw()

        # 5. Searchable Log (Treeview)
        self.tree.delete(*self.tree.get_children())
        # Cap display at 10k to prevent Tkinter freezing on massive traces
        display_events = d["events"][:10000] 
        for ev in display_events:
            self.tree.insert("", tk.END, values=ev)
        if len(d["events"]) > 10000:
            self.tree.insert("", tk.END, values=("...", "...", "TRUNCATED", "Showing first 10,000", "Export CSV for full log", ""))

        # 6. Anomalies Text
        self.anomaly_text.delete(1.0, tk.END)
        self.anomaly_text.insert(tk.END, f"--- LEAK REPORT ({len(d['leaks'])} found) ---\n\n")
        self.anomaly_text.insert(tk.END, "\n".join(d['leaks']) if d['leaks'] else "No leaks detected.\n")
        self.anomaly_text.insert(tk.END, f"\n\n--- INVALID FREE REPORT ({len(d['invalid_frees'])} found) ---\n\n")
        self.anomaly_text.insert(tk.END, "\n".join(d['invalid_frees']) if d['invalid_frees'] else "No invalid frees detected.\n")

        # 7. Raw Report
        self.report_text.delete(1.0, tk.END)
        self.report_text.insert(tk.END, "\n".join(d["report_lines"]))

    def filter_events(self, *args):
        if not self.data_cache: return
        query = self.search_var.get().lower()
        self.tree.delete(*self.tree.get_children())
        
        count = 0
        for ev in self.data_cache["events"]:
            # ev format: (step, thread, op, size, mem, raw)
            if query in str(ev).lower():
                self.tree.insert("", tk.END, values=ev)
                count += 1
                if count > 5000: # Limit filtered results display speed
                    self.tree.insert("", tk.END, values=("...", "...", "LIMIT", "Refine search...", "", ""))
                    break

    def export_csv(self):
        if not self.data_cache or not self.data_cache["events"]:
            return
            
        filepath = filedialog.asksaveasfilename(
            defaultextension=".csv",
            title="Export Event Log to CSV",
            filetypes=[("CSV Files", "*.csv")]
        )
        if not filepath:
            return
            
        try:
            with open(filepath, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["Step", "Thread_ID", "Operation", "Size_Bytes", "Total_Memory_Bytes", "Raw_Log"])
                for ev in self.data_cache["events"]:
                    writer.writerow(ev)
            messagebox.showinfo("Export Successful", f"Log successfully exported to {os.path.basename(filepath)}")
        except Exception as e:
            messagebox.showerror("Export Error", f"Failed to export CSV:\n{str(e)}")


if __name__ == "__main__":
    root = tk.Tk()
    app = MemoryAnalyzerGUI(root)
    root.mainloop()