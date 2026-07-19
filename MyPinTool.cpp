// // version----1---------------------
// // #include "pin.H"
// // #include <iostream>
// // #include <fstream>
// // #include <unordered_map>
// // #include <string>

// // // =====================================================================
// // // Global Variables & Data Structures
// // // =====================================================================

// // // Knob for output trace file name
// // KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memory_trace.out", "Specify output file name");
// // std::ofstream TraceFile;

// // // Thread-safe lock for our global data structures
// // PIN_LOCK map_lock;

// // // Allocation record
// // struct AllocRecord {
// //     size_t size;
// //     ADDRINT callerIp;
// // };

// // // Map to track active allocations: pointer -> AllocRecord
// // std::unordered_map<void*, AllocRecord> active_allocs;

// // // Statistics
// // UINT64 total_allocated = 0;
// // UINT64 total_freed = 0;
// // UINT64 current_memory = 0;
// // UINT64 peak_memory = 0;

// // // Anomaly Counters
// // UINT64 double_frees = 0;
// // UINT64 invalid_frees = 0;

// // // =====================================================================
// // // Wrapper Functions (Replaced Signatures)
// // // =====================================================================

// // // 1. MALLOC WRAPPER
// // void* MallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t size) {
// //     // Call the original malloc
// //     void* ptr = nullptr;
// //     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), size, PIN_PARG_END());

// //     if (ptr != nullptr) {
// //         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR); // Get caller address
        
// //         PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
// //         active_allocs[ptr] = {size, callerIp};
// //         total_allocated += size;
// //         current_memory += size;
// //         if (current_memory > peak_memory) peak_memory = current_memory;
        
// //         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (malloc) : Size=" << size 
// //                   << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
// //         PIN_ReleaseLock(&map_lock);
// //     }
// //     return ptr;
// // }

// // // 2. FREE WRAPPER
// // void FreeWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr) {
// //     if (ptr == nullptr) {
// //         // Freeing NULL is a no-op in standard C, but we log it.
// //         PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
// //         return;
// //     }

// //     PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
// //     auto it = active_allocs.find(ptr);
    
// //     if (it != active_allocs.end()) {
// //         // Valid Free
// //         size_t size = it->second.size;
// //         total_freed += size;
// //         current_memory -= size;
// //         active_allocs.erase(it);
        
// //         TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (free)   : Size=" << size 
// //                   << " bytes | Addr=" << ptr << "\n";
                  
// //         PIN_ReleaseLock(&map_lock);
// //         // Execute original free
// //         PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
// //     } else {
// //         // Anomaly Detected! Either Double Free or Invalid Free
// //         TraceFile << "[ERROR] INVALID/DOUBLE FREE Detected! Addr=" << ptr << "\n";
// //         invalid_frees++;
// //         PIN_ReleaseLock(&map_lock);
// //         // Note: We skip calling original free to prevent application crash during analysis
// //     }
// // }

// // // 3. CALLOC WRAPPER
// // void* CallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t num, size_t size) {
// //     void* ptr = nullptr;
// //     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), num, PIN_PARG(size_t), size, PIN_PARG_END());

// //     size_t total_size = num * size;
// //     if (ptr != nullptr && total_size > 0) {
// //         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
        
// //         PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
// //         active_allocs[ptr] = {total_size, callerIp};
// //         total_allocated += total_size;
// //         current_memory += total_size;
// //         if (current_memory > peak_memory) peak_memory = current_memory;
        
// //         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (calloc) : Size=" << total_size 
// //                   << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
// //         PIN_ReleaseLock(&map_lock);
// //     }
// //     return ptr;
// // }

// // // 4. REALLOC WRAPPER
// // void* ReallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr, size_t new_size) {
// //     void* new_ptr = nullptr;
// //     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &new_ptr, PIN_PARG(void*), ptr, PIN_PARG(size_t), new_size, PIN_PARG_END());

// //     PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
    
// //     // Handle the "free" part of realloc
// //     if (ptr != nullptr) {
// //         auto it = active_allocs.find(ptr);
// //         if (it != active_allocs.end()) {
// //             current_memory -= it->second.size;
// //             total_freed += it->second.size;
// //             TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (realloc): Size=" << it->second.size 
// //                       << " bytes | OldAddr=" << ptr << "\n";
// //             active_allocs.erase(it);
// //         } else {
// //             TraceFile << "[ERROR] REALLOC INVALID FREE Detected! Addr=" << ptr << "\n";
// //             invalid_frees++;
// //         }
// //     }

// //     // Handle the "alloc" part of realloc
// //     if (new_ptr != nullptr && new_size > 0) {
// //         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
// //         active_allocs[new_ptr] = {new_size, callerIp};
// //         total_allocated += new_size;
// //         current_memory += new_size;
// //         if (current_memory > peak_memory) peak_memory = current_memory;
        
// //         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (realloc): Size=" << new_size 
// //                   << " bytes | NewAddr=" << new_ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
// //     }
    
// //     PIN_ReleaseLock(&map_lock);
// //     return new_ptr;
// // }

// // // =====================================================================
// // // Instrumentation Routines
// // // =====================================================================

// // VOID ImageLoad(IMG img, VOID *v) {
// //     // Optional: Filter to only instrument the main executable if required
// //     // if (!IMG_IsMainExecutable(img)) return;

// //     // Find and replace malloc
// //     RTN mallocRtn = RTN_FindByName(img, "malloc");
// //     if (RTN_Valid(mallocRtn)) {
// //         PROTO protoMalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());
// //         RTN_ReplaceSignature(mallocRtn, AFUNPTR(MallocWrapper),
// //             IARG_PROTOTYPE, protoMalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
// //         PROTO_Free(protoMalloc);
// //     }

// //     // Find and replace free
// //     RTN freeRtn = RTN_FindByName(img, "free");
// //     if (RTN_Valid(freeRtn)) {
// //         PROTO protoFree = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "free", PIN_PARG(void*), PIN_PARG_END());
// //         RTN_ReplaceSignature(freeRtn, AFUNPTR(FreeWrapper),
// //             IARG_PROTOTYPE, protoFree, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
// //         PROTO_Free(protoFree);
// //     }

// //     // Find and replace calloc
// //     RTN callocRtn = RTN_FindByName(img, "calloc");
// //     if (RTN_Valid(callocRtn)) {
// //         PROTO protoCalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "calloc", PIN_PARG(size_t), PIN_PARG(size_t), PIN_PARG_END());
// //         RTN_ReplaceSignature(callocRtn, AFUNPTR(CallocWrapper),
// //             IARG_PROTOTYPE, protoCalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
// //         PROTO_Free(protoCalloc);
// //     }

// //     // Find and replace realloc
// //     RTN reallocRtn = RTN_FindByName(img, "realloc");
// //     if (RTN_Valid(reallocRtn)) {
// //         PROTO protoRealloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "realloc", PIN_PARG(void*), PIN_PARG(size_t), PIN_PARG_END());
// //         RTN_ReplaceSignature(reallocRtn, AFUNPTR(ReallocWrapper),
// //             IARG_PROTOTYPE, protoRealloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
// //         PROTO_Free(protoRealloc);
// //     }
// // }

// // // =====================================================================
// // // Final Checker & Reporting (Fini)
// // // =====================================================================

// // VOID Fini(INT32 code, VOID *v) {
// //     TraceFile << "\n===========================================================\n";
// //     TraceFile << "               MEMORY ANALYSIS FINAL REPORT                  \n";
// //     TraceFile << "===========================================================\n\n";

// //     TraceFile << "--- GLOBAL STATISTICS ---\n";
// //     TraceFile << "Total Memory Allocated : " << total_allocated << " bytes\n";
// //     TraceFile << "Total Memory Freed     : " << total_freed << " bytes\n";
// //     TraceFile << "Peak Memory Usage      : " << peak_memory << " bytes\n";
// //     TraceFile << "Memory Leaked          : " << current_memory << " bytes\n\n";

// //     TraceFile << "--- ANOMALY DETECTION ---\n";
// //     TraceFile << "Invalid / Double Frees Detected: " << invalid_frees << "\n\n";

// //     TraceFile << "--- MEMORY LEAK SUMMARY ---\n";
// //     if (active_allocs.empty()) {
// //         TraceFile << "STATUS: CLEAN. No memory leaks detected!\n";
// //     } else {
// //         TraceFile << "STATUS: LEAKS DETECTED! (" << active_allocs.size() << " unfreed blocks)\n";
// //         for (auto const& pair : active_allocs) {
// //             TraceFile << "  -> Leaked Address: " << pair.first 
// //                       << " | Size: " << pair.second.size << " bytes"
// //                       << " | CallerIP: 0x" << std::hex << pair.second.callerIp << std::dec << "\n";
// //         }
// //     }

// //     TraceFile << "\n===========================================================\n";
// //     TraceFile.close();
// // }

// // // =====================================================================
// // // Main
// // // =====================================================================

// // int main(int argc, char *argv[]) {
// //     // Initialize PIN
// //     PIN_InitSymbols();
// //     if (PIN_Init(argc, argv)) {
// //         std::cerr << "Command line error\n";
// //         return -1;
// //     }

// //     // Initialize Thread Lock
// //     PIN_InitLock(&map_lock);

// //     // Open Output File
// //     TraceFile.open(KnobOutputFile.Value().c_str());

// //     // Register ImageLoad to find memory functions
// //     IMG_AddInstrumentFunction(ImageLoad, 0);

// //     // Register Fini to generate final report
// //     PIN_AddFiniFunction(Fini, 0);

// //     // Start program execution
// //     PIN_StartProgram();
// //     return 0;
// // }



// // //version-2---------------------------------------------------:invalid free double free integarion
// #include "pin.H"
// #include <iostream>
// #include <fstream>
// #include <unordered_map>
// #include <unordered_set>
// #include <string>

// // =====================================================================
// // Global Variables & Data Structures
// // =====================================================================

// // Knob for output trace file name
// KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memory_trace.out", "Specify output file name");
// std::ofstream TraceFile;

// // Thread-safe lock for our global data structures
// PIN_LOCK map_lock;

// // Allocation record
// struct AllocRecord {
//     size_t size;
//     ADDRINT callerIp;
// };

// // Map to track currently active allocations: pointer -> AllocRecord
// std::unordered_map<void*, AllocRecord> active_allocs;

// // Set to track the history of all legally freed pointers
// std::unordered_set<void*> freed_allocs;

// // Global Statistics
// UINT64 total_allocated = 0;
// UINT64 total_freed = 0;
// UINT64 current_memory = 0;
// UINT64 peak_memory = 0;

// // Anomaly Counters
// UINT64 double_frees = 0;
// UINT64 invalid_frees = 0;

// // =====================================================================
// // Wrapper Functions (Replaced Signatures)
// // =====================================================================

// // 1. MALLOC WRAPPER
// void* MallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t size) {
//     void* ptr = nullptr;
//     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), size, PIN_PARG_END());

//     if (ptr != nullptr) {
//         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR); 
        
//         PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
//         active_allocs[ptr] = {size, callerIp};
//         total_allocated += size;
//         current_memory += size;
//         if (current_memory > peak_memory) peak_memory = current_memory;
        
//         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (malloc) : Size=" << size 
//                   << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
//         PIN_ReleaseLock(&map_lock);
//     }
//     return ptr;
// }

// // 2. FREE WRAPPER
// void FreeWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr) {
//     if (ptr == nullptr) {
//         // Freeing NULL is a no-op in C, execute original without logging
//         PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
//         return;
//     }

//     PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
//     auto it = active_allocs.find(ptr);
    
//     if (it != active_allocs.end()) {
//         // --- VALID FREE ---
//         size_t size = it->second.size;
//         total_freed += size;
//         current_memory -= size;
        
//         active_allocs.erase(it);
//         freed_allocs.insert(ptr); // Add to history
        
//         TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (free)    : Size=" << size 
//                   << " bytes | Addr=" << ptr << "\n";
                  
//         PIN_ReleaseLock(&map_lock);
//         PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
    
//     } else if (freed_allocs.find(ptr) != freed_allocs.end()) {
//         // --- DOUBLE FREE DETECTED ---
//         TraceFile << "[ERROR] DOUBLE FREE Detected! Addr=" << ptr << "\n";
//         double_frees++;
//         PIN_ReleaseLock(&map_lock);
//         // Skip calling original free to prevent OS crash
        
//     } else {
//         // --- INVALID FREE DETECTED ---
//         TraceFile << "[ERROR] INVALID FREE Detected! Addr=" << ptr << " (Not a heap pointer)\n";
//         invalid_frees++;
//         PIN_ReleaseLock(&map_lock);
//         // Skip calling original free to prevent OS crash
//     }
// }

// // 3. CALLOC WRAPPER
// void* CallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t num, size_t size) {
//     void* ptr = nullptr;
//     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), num, PIN_PARG(size_t), size, PIN_PARG_END());

//     size_t total_size = num * size;
//     if (ptr != nullptr && total_size > 0) {
//         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
        
//         PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
//         active_allocs[ptr] = {total_size, callerIp};
//         total_allocated += total_size;
//         current_memory += total_size;
//         if (current_memory > peak_memory) peak_memory = current_memory;
        
//         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (calloc) : Size=" << total_size 
//                   << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
//         PIN_ReleaseLock(&map_lock);
//     }
//     return ptr;
// }

// // 4. REALLOC WRAPPER
// void* ReallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr, size_t new_size) {
//     void* new_ptr = nullptr;
//     PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &new_ptr, PIN_PARG(void*), ptr, PIN_PARG(size_t), new_size, PIN_PARG_END());

//     PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
    
//     // Step A: Handle the freeing of the old pointer
//     if (ptr != nullptr) {
//         auto it = active_allocs.find(ptr);
//         if (it != active_allocs.end()) {
//             current_memory -= it->second.size;
//             total_freed += it->second.size;
//             TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (realloc): Size=" << it->second.size 
//                       << " bytes | OldAddr=" << ptr << "\n";
            
//             active_allocs.erase(it);
//             freed_allocs.insert(ptr); 
//         } else if (freed_allocs.find(ptr) != freed_allocs.end()) {
//             TraceFile << "[ERROR] REALLOC DOUBLE FREE! Addr=" << ptr << "\n";
//             double_frees++;
//         } else {
//             TraceFile << "[ERROR] REALLOC INVALID FREE! Addr=" << ptr << "\n";
//             invalid_frees++;
//         }
//     }

//     // Step B: Handle the allocation of the new pointer
//     if (new_ptr != nullptr && new_size > 0) {
//         ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
//         active_allocs[new_ptr] = {new_size, callerIp};
//         total_allocated += new_size;
//         current_memory += new_size;
//         if (current_memory > peak_memory) peak_memory = current_memory;
        
//         TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (realloc): Size=" << new_size 
//                   << " bytes | NewAddr=" << new_ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
//     }
    
//     PIN_ReleaseLock(&map_lock);
//     return new_ptr;
// }

// // =====================================================================
// // Instrumentation Routines
// // =====================================================================

// VOID ImageLoad(IMG img, VOID *v) {
//     RTN mallocRtn = RTN_FindByName(img, "malloc");
//     if (RTN_Valid(mallocRtn)) {
//         PROTO protoMalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());
//         RTN_ReplaceSignature(mallocRtn, AFUNPTR(MallocWrapper), IARG_PROTOTYPE, protoMalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
//         PROTO_Free(protoMalloc);
//     }

//     RTN freeRtn = RTN_FindByName(img, "free");
//     if (RTN_Valid(freeRtn)) {
//         PROTO protoFree = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "free", PIN_PARG(void*), PIN_PARG_END());
//         RTN_ReplaceSignature(freeRtn, AFUNPTR(FreeWrapper), IARG_PROTOTYPE, protoFree, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
//         PROTO_Free(protoFree);
//     }

//     RTN callocRtn = RTN_FindByName(img, "calloc");
//     if (RTN_Valid(callocRtn)) {
//         PROTO protoCalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "calloc", PIN_PARG(size_t), PIN_PARG(size_t), PIN_PARG_END());
//         RTN_ReplaceSignature(callocRtn, AFUNPTR(CallocWrapper), IARG_PROTOTYPE, protoCalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
//         PROTO_Free(protoCalloc);
//     }

//     RTN reallocRtn = RTN_FindByName(img, "realloc");
//     if (RTN_Valid(reallocRtn)) {
//         PROTO protoRealloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "realloc", PIN_PARG(void*), PIN_PARG(size_t), PIN_PARG_END());
//         RTN_ReplaceSignature(reallocRtn, AFUNPTR(ReallocWrapper), IARG_PROTOTYPE, protoRealloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
//         PROTO_Free(protoRealloc);
//     }
// }

// // =====================================================================
// // Final Checker & Reporting (Fini)
// // =====================================================================

// VOID Fini(INT32 code, VOID *v) {
//     TraceFile << "\n===========================================================\n";
//     TraceFile << "               MEMORY ANALYSIS FINAL REPORT                  \n";
//     TraceFile << "===========================================================\n\n";

//     TraceFile << "--- GLOBAL STATISTICS ---\n";
//     TraceFile << "Total Memory Allocated : " << total_allocated << " bytes\n";
//     TraceFile << "Total Memory Freed     : " << total_freed << " bytes\n";
//     TraceFile << "Peak Memory Usage      : " << peak_memory << " bytes\n";
//     TraceFile << "Memory Leaked          : " << current_memory << " bytes\n\n";

//     TraceFile << "--- ANOMALY DETECTION ---\n";
//     TraceFile << "Double Frees Detected  : " << double_frees << "\n";
//     TraceFile << "Invalid Frees Detected : " << invalid_frees << "\n\n";

//     TraceFile << "--- MEMORY LEAK SUMMARY ---\n";
//     if (active_allocs.empty()) {
//         TraceFile << "STATUS: CLEAN. No memory leaks detected!\n";
//     } else {
//         TraceFile << "STATUS: LEAKS DETECTED! (" << active_allocs.size() << " unfreed blocks)\n";
//         for (auto const& pair : active_allocs) {
//             TraceFile << "  -> Leaked Address: " << pair.first 
//                       << " | Size: " << pair.second.size << " bytes"
//                       << " | CallerIP: 0x" << std::hex << pair.second.callerIp << std::dec << "\n";
//         }
//     }

//     TraceFile << "\n===========================================================\n";
//     TraceFile.close();
// }

// // =====================================================================
// // Main
// // =====================================================================

// int main(int argc, char *argv[]) {
//     PIN_InitSymbols();
//     if (PIN_Init(argc, argv)) {
//         std::cerr << "Command line error\n";
//         return -1;
//     }

//     PIN_InitLock(&map_lock);
//     TraceFile.open(KnobOutputFile.Value().c_str());

//     IMG_AddInstrumentFunction(ImageLoad, 0);
//     PIN_AddFiniFunction(Fini, 0);

//     PIN_StartProgram();
//     return 0;
// }


// version 3 ----------Global lock is integrated with tls -----------------
#include "pin.H"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>

// =====================================================================
// Global Variables & Data Structures
// =====================================================================

KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memory_trace.out", "Specify output file name");
std::ofstream TraceFile;
PIN_LOCK map_lock;

// Map and Set MUST remain global because Thread A can allocate and Thread B can free
struct AllocRecord {
    size_t size;
    ADDRINT callerIp;
};
std::unordered_map<void*, AllocRecord> active_allocs;
std::unordered_set<void*> freed_allocs;

// Global memory states (Updated safely inside locks)
UINT64 total_allocated = 0;
UINT64 total_freed = 0;
UINT64 current_memory = 0;
UINT64 peak_memory = 0;
UINT64 double_frees = 0;
UINT64 invalid_frees = 0;

// =====================================================================
// Thread Local Storage (TLS) Setup
// =====================================================================

TLS_KEY tls_key = INVALID_TLS_KEY;

// The "Personal Cash Drawer" for each thread like in medical shop each medicine compnay or each constituent has its own box or space
struct ThreadData {
    UINT64 thread_bytes_allocated = 0;
    UINT64 thread_bytes_freed = 0;
    UINT64 thread_alloc_calls = 0;
    UINT64 thread_free_calls = 0;
};

// Called when a new thread starts
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    ThreadData* tdata = new ThreadData();
    PIN_SetThreadData(tls_key, tdata, threadid);
}

// Called when a thread finishes
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v) {
    ThreadData* tdata = static_cast<ThreadData*>(PIN_GetThreadData(tls_key, threadid));
    if (tdata != nullptr) {
        PIN_GetLock(&map_lock, threadid + 1);
        TraceFile << "\n[THREAD " << threadid << " SUMMARY]\n"
                  << "  -> Allocs: " << tdata->thread_alloc_calls << "\n"
                  << "  -> Frees : " << tdata->thread_free_calls << "\n"
                  << "  -> Bytes Allocated: " << tdata->thread_bytes_allocated << " bytes\n"
                  << "  -> Bytes Freed    : " << tdata->thread_bytes_freed << " bytes\n";
        PIN_ReleaseLock(&map_lock);
        
        delete tdata;
        PIN_SetThreadData(tls_key, nullptr, threadid);
    }
}

// =====================================================================
// Wrapper Functions (Optimized with TLS)
// =====================================================================

// 1. MALLOC
void* MallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t size) {
    void* ptr = nullptr;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), size, PIN_PARG_END());

    if (ptr != nullptr) {
        ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR); 
        
        // Safely update Lock-Free TLS Data
        ThreadData* tdata = static_cast<ThreadData*>(PIN_GetThreadData(tls_key, PIN_ThreadId()));
        if (tdata != nullptr) {
            tdata->thread_bytes_allocated += size;
            tdata->thread_alloc_calls++;
        }

        // Minimal Locked Section for global data
        PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
        active_allocs[ptr] = {size, callerIp};
        total_allocated += size;
        current_memory += size;
        if (current_memory > peak_memory) peak_memory = current_memory;
        
        TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (malloc) : Size=" << size 
                  << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
        PIN_ReleaseLock(&map_lock);
    }
    return ptr;
}

// 2. CALLOC
void* CallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, size_t num, size_t size) {
    void* ptr = nullptr;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &ptr, PIN_PARG(size_t), num, PIN_PARG(size_t), size, PIN_PARG_END());

    size_t total_size = num * size;
    if (ptr != nullptr && total_size > 0) {
        ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
        
        ThreadData* tdata = static_cast<ThreadData*>(PIN_GetThreadData(tls_key, PIN_ThreadId()));
        if (tdata != nullptr) {
            tdata->thread_bytes_allocated += total_size;
            tdata->thread_alloc_calls++;
        }
        
        PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
        active_allocs[ptr] = {total_size, callerIp};
        total_allocated += total_size;
        current_memory += total_size;
        if (current_memory > peak_memory) peak_memory = current_memory;
        
        TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (calloc) : Size=" << total_size 
                  << " bytes | Addr=" << ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
        PIN_ReleaseLock(&map_lock);
    }
    return ptr;
}

// 3. FREE
void FreeWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr) {
    if (ptr == nullptr) {
        PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
        return;
    }

    ThreadData* tdata = static_cast<ThreadData*>(PIN_GetThreadData(tls_key, PIN_ThreadId()));
    if (tdata != nullptr) {
        tdata->thread_free_calls++;
    }

    PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
    auto it = active_allocs.find(ptr);
    
    if (it != active_allocs.end()) {
        size_t size = it->second.size;
        current_memory -= size;
        total_freed += size;
        
        if (tdata != nullptr) tdata->thread_bytes_freed += size; 
        
        active_allocs.erase(it);
        freed_allocs.insert(ptr);
        
        TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (free)    : Size=" << size 
                  << " bytes | Addr=" << ptr << "\n";
                  
        PIN_ReleaseLock(&map_lock);
        PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void), PIN_PARG(void*), ptr, PIN_PARG_END());
    
    } else if (freed_allocs.find(ptr) != freed_allocs.end()) {
        TraceFile << "[ERROR] DOUBLE FREE Detected! Addr=" << ptr << "\n";
        double_frees++;
        PIN_ReleaseLock(&map_lock);
    } else {
        TraceFile << "[ERROR] INVALID FREE Detected! Addr=" << ptr << "\n";
        invalid_frees++;
        PIN_ReleaseLock(&map_lock);
    }
}

// 4. REALLOC
void* ReallocWrapper(CONTEXT *ctxt, AFUNPTR orgFunc, void* ptr, size_t new_size) {
    void* new_ptr = nullptr;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFunc, NULL, PIN_PARG(void*), &new_ptr, PIN_PARG(void*), ptr, PIN_PARG(size_t), new_size, PIN_PARG_END());

    ThreadData* tdata = static_cast<ThreadData*>(PIN_GetThreadData(tls_key, PIN_ThreadId()));
    
    PIN_GetLock(&map_lock, PIN_ThreadId() + 1);
    
    // Free part
    if (ptr != nullptr) {
        if (tdata != nullptr) tdata->thread_free_calls++;
        
        auto it = active_allocs.find(ptr);
        if (it != active_allocs.end()) {
            current_memory -= it->second.size;
            total_freed += it->second.size;
            if (tdata != nullptr) tdata->thread_bytes_freed += it->second.size;
            
            TraceFile << "[Thread " << PIN_ThreadId() << "] FREE (realloc): Size=" << it->second.size 
                      << " bytes | OldAddr=" << ptr << "\n";
            active_allocs.erase(it);
            freed_allocs.insert(ptr); 
        } else if (freed_allocs.find(ptr) != freed_allocs.end()) {
            TraceFile << "[ERROR] REALLOC DOUBLE FREE! Addr=" << ptr << "\n";
            double_frees++;
        } else {
            TraceFile << "[ERROR] REALLOC INVALID FREE! Addr=" << ptr << "\n";
            invalid_frees++;
        }
    }

    // Alloc part
    if (new_ptr != nullptr && new_size > 0) {
        if (tdata != nullptr) {
            tdata->thread_alloc_calls++;
            tdata->thread_bytes_allocated += new_size;
        }
        
        ADDRINT callerIp = PIN_GetContextReg(ctxt, REG_INST_PTR);
        active_allocs[new_ptr] = {new_size, callerIp};
        total_allocated += new_size;
        current_memory += new_size;
        if (current_memory > peak_memory) peak_memory = current_memory;
        
        TraceFile << "[Thread " << PIN_ThreadId() << "] ALLOC (realloc): Size=" << new_size 
                  << " bytes | NewAddr=" << new_ptr << " | CallerIP=0x" << std::hex << callerIp << std::dec << "\n";
    }
    
    PIN_ReleaseLock(&map_lock);
    return new_ptr;
}

// =====================================================================
// Instrumentation Routines
// =====================================================================

VOID ImageLoad(IMG img, VOID *v) {
    RTN mallocRtn = RTN_FindByName(img, "malloc");
    if (RTN_Valid(mallocRtn)) {
        PROTO protoMalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());
        RTN_ReplaceSignature(mallocRtn, AFUNPTR(MallocWrapper), IARG_PROTOTYPE, protoMalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        PROTO_Free(protoMalloc);
    }

    RTN freeRtn = RTN_FindByName(img, "free");
    if (RTN_Valid(freeRtn)) {
        PROTO protoFree = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "free", PIN_PARG(void*), PIN_PARG_END());
        RTN_ReplaceSignature(freeRtn, AFUNPTR(FreeWrapper), IARG_PROTOTYPE, protoFree, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        PROTO_Free(protoFree);
    }

    RTN callocRtn = RTN_FindByName(img, "calloc");
    if (RTN_Valid(callocRtn)) {
        PROTO protoCalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "calloc", PIN_PARG(size_t), PIN_PARG(size_t), PIN_PARG_END());
        RTN_ReplaceSignature(callocRtn, AFUNPTR(CallocWrapper), IARG_PROTOTYPE, protoCalloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
        PROTO_Free(protoCalloc);
    }

    RTN reallocRtn = RTN_FindByName(img, "realloc");
    if (RTN_Valid(reallocRtn)) {
        PROTO protoRealloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "realloc", PIN_PARG(void*), PIN_PARG(size_t), PIN_PARG_END());
        RTN_ReplaceSignature(reallocRtn, AFUNPTR(ReallocWrapper), IARG_PROTOTYPE, protoRealloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
        PROTO_Free(protoRealloc);
    }
}

// =====================================================================
// Final Checker & Reporting (Fini)
// =====================================================================

VOID Fini(INT32 code, VOID *v) {
    TraceFile << "\n===========================================================\n";
    TraceFile << "               MEMORY ANALYSIS FINAL REPORT                  \n";
    TraceFile << "===========================================================\n\n";

    TraceFile << "--- GLOBAL STATISTICS ---\n";
    TraceFile << "Total Memory Allocated : " << total_allocated << " bytes\n";
    TraceFile << "Total Memory Freed     : " << total_freed << " bytes\n";
    TraceFile << "Peak Memory Usage      : " << peak_memory << " bytes\n";
    TraceFile << "Memory Leaked          : " << current_memory << " bytes\n\n";

    TraceFile << "--- ANOMALY DETECTION ---\n";
    TraceFile << "Double Frees Detected  : " << double_frees << "\n";
    TraceFile << "Invalid Frees Detected : " << invalid_frees << "\n\n";

    TraceFile << "--- MEMORY LEAK SUMMARY ---\n";
    if (active_allocs.empty()) {
        TraceFile << "STATUS: CLEAN. No memory leaks detected!\n";
    } else {
        TraceFile << "STATUS: LEAKS DETECTED! (" << active_allocs.size() << " unfreed blocks)\n";
        for (auto const& pair : active_allocs) {
            TraceFile << "  -> Leaked Address: " << pair.first 
                      << " | Size: " << pair.second.size << " bytes"
                      << " | CallerIP: 0x" << std::hex << pair.second.callerIp << std::dec << "\n";
        }
    }

    TraceFile << "\n===========================================================\n";
    TraceFile.close();
}

// =====================================================================
// Main
// =====================================================================

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) {
        std::cerr << "Command line error\n";
        return -1;
    }

    PIN_InitLock(&map_lock);
    
    // Request a TLS key from the PIN engine
    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY) {
        std::cerr << "Failed to allocate TLS key!\n";
        return -1;
    }

    TraceFile.open(KnobOutputFile.Value().c_str());

    // Register Callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0); // THIS IS WHAT WAS MISSING!
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();
    return 0;
}