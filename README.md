
# Boot Log Analyzer with OpenMP

A parallel and non-parallel log analysis framework written in **C**, leveraging **OpenMP** for performance. This tool parses system boot log files, detects anomalies using PCRE-based pattern matching, and stores issue-specific reports in separate output files.

---

##  Project Structure

```bash
.
├── get_logs.sh              # Script to extract boot logs via journalctl
├── lines.py                 # Python script to count lines in logs
├── non_parallel             # Non-parallel version of the analyzer
│   ├── compile.sh           # Compilation script
│   ├── lines.py             # Line count utility
│   ├── Log_Analyzer
│   │   ├── analyzer.c       # Log analyzer using PCRE (serial)
│   │   ├── compile.sh       # Compilation script
│   │   └── lines.py         # Local line checker
│   └── log_parser.c         # Boot log parser (serial)
├── parallel                 # Parallel version using OpenMP
│   ├── compile.sh           # Compilation script with -fopenmp
│   ├── lines.py             # Line count utility
│   ├── Log_Analyzer
│   │   ├── analyzer.c       # Parallel analyzer (OpenMP + PCRE)
│   │   └── compile.sh       # Compilation script
│   └── log_parser.c         # Boot log parser (OpenMP)
└── test.c                  
```

---

##  Requirements

* GCC with OpenMP support
* PCRE library

### Install Dependencies on Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libpcre3-dev
```

---

##  How to Use
### Step 1: Clone the Repo
```
git clone https://github.com/ayushadarsh7/concurrent-log-analyzer
```

### Step 2: Extract `boot.log`

```bash
./get_logs.sh
```

This runs `journalctl` and extracts logs into `boot.log`.

### Step 3: Run the Parser

Navigate to either the `parallel/` or `non_parallel/` folder.

```bash
cd parallel
./compile.sh        # compiles parser and parses logs categorically in the directory  
cd Log_Analyzer
./compile.sh        # analyses various logs, flags the various issues & threats and stores them in individual files categorically, in the same directory.
```

### Step 3: View Outputs
* Move to Log_Analyzer sub-directory of parallel or non-parallel directory
Output files like `authentication_issues.log`, `mount_fs_issues.log`, etc., will be generated in `Log_Analyzer/`.

### Optional: Count Lines in Output Files

```bash
python3 lines.py
```



##  Parallelism with OpenMP

The parallel version uses:

```c
#define NTHREADS 8
omp_set_num_threads(NTHREADS);
```

Each thread analyzes one type of log in parallel. Time is measured using `omp_get_wtime()` and logged in `time_taken.txt`.

You can clearly see the difference in time taken by parallel and non-parallel(serialised) log parsing & analysis through the various time_taken.txt files in directories.
It clearly shows that parallel algorithm takes considerably less time compared to serialised algorithm in log parsing and analysis.

---


This project is licensed under the MIT License.
