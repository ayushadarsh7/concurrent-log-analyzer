file_paths = ["boot.log","authentication.log","critical_errors.log","failed_services.log","hardware_driver.log","mount_fs.log","networking.log","startup_timing.log","warnings.log"]



for file_path in file_paths:
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
        line_count = sum(1 for _ in file)
        print(f"Total number of lines in {file_path} : {line_count}")
