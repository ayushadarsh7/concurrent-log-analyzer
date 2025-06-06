file_paths = ["authentication_issues.log","critical_errors_issues.log","failed_services_issues.log","hardware_driver_issues.log","mount_fs_issues.log","networking_issues.log","startup_timing_issues.log","warnings_issues.log"]



for file_path in file_paths:
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
            line_count = sum(1 for _ in file)
            print(f"Total number of lines in {file_path} : {line_count}")
    except FileNotFoundError:
        print(f"No such file: {file_path}")