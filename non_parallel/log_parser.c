#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // for strcasestr()
#include <omp.h>       // for omp_get_wtime()

#define MAX_LINE_LEN 8192

/* 
 * Each function checks its own set of keywords in 'line'.
 * If a keyword is found (case‐insensitive), it writes 'line' into 'out'.
 */

/* 1. Startup timing: look for "Startup finished in" or "Reached target" */
void parse_startup_timing(const char *line, FILE *out) {
    if (strcasestr(line, "Startup finished in") != NULL ||
        strcasestr(line, "Reached target") != NULL) {
        fputs(line, out);
    }
}

/* 2. Failed services: look for "failed" */
void parse_failed_services(const char *line, FILE *out) {
    if (strcasestr(line, "failed") != NULL) {
        fputs(line, out);
    }
}

/* 3. Warnings: look for "warn" */
void parse_warnings(const char *line, FILE *out) {
    if (strcasestr(line, "warn") != NULL) {
        fputs(line, out);
    }
}

/* 4. Critical errors: look for "error" or "critical" */
void parse_critical_errors(const char *line, FILE *out) {
    if (strcasestr(line, "error") != NULL ||
        strcasestr(line, "critical") != NULL) {
        fputs(line, out);
    }
}

/* 5. Hardware/Driver logs: look for any of "usb", "pci", "sda", "nvme", "driver", "firmware" */
void parse_hardware_driver(const char *line, FILE *out) {
    if (strcasestr(line, "usb")     != NULL ||
        strcasestr(line, "pci")     != NULL ||
        strcasestr(line, "sda")     != NULL ||
        strcasestr(line, "nvme")    != NULL ||
        strcasestr(line, "driver")  != NULL ||
        strcasestr(line, "firmware")!= NULL) {
        fputs(line, out);
    }
}

/* 6. Networking logs: look for "network", "eth0", "wlan", "dhcp", "ip" */
void parse_networking(const char *line, FILE *out) {
    if (strcasestr(line, "network") != NULL ||
        strcasestr(line, "eth0")    != NULL ||
        strcasestr(line, "wlan")    != NULL ||
        strcasestr(line, "dhcp")    != NULL ||
        strcasestr(line, "ip")      != NULL) {
        fputs(line, out);
    }
}

/* 7. Authentication logs: look for "sudo", "authentication", "passwd", "login" */
void parse_authentication(const char *line, FILE *out) {
    if (strcasestr(line, "sudo")           != NULL ||
        strcasestr(line, "authentication") != NULL ||
        strcasestr(line, "passwd")         != NULL ||
        strcasestr(line, "login")          != NULL) {
        fputs(line, out);
    }
}

/* 8. Mount and filesystem issues: look for "mount", "fsck", "ext4", "btrfs", "ntfs" */
void parse_mount_fs(const char *line, FILE *out) {
    if (strcasestr(line, "mount") != NULL ||
        strcasestr(line, "fsck")  != NULL ||
        strcasestr(line, "ext4")  != NULL ||
        strcasestr(line, "btrfs") != NULL ||
        strcasestr(line, "ntfs")  != NULL) {
        fputs(line, out);
    }
}


int main(void) {
    /* Record start time (in seconds) */
    double start_time = omp_get_wtime();

    FILE *fin = fopen("../boot.log", "r");
    if (!fin) {
        perror("Error opening boot.log");
        return EXIT_FAILURE;
    }

    /* Open one output file per category */
    FILE *f_startup     = fopen("startup_timing.log",     "w");
    FILE *f_failed      = fopen("failed_services.log",    "w");
    FILE *f_warnings    = fopen("warnings.log",           "w");
    FILE *f_crit_errors = fopen("critical_errors.log",    "w");
    FILE *f_hw_driver   = fopen("hardware_driver.log",    "w");
    FILE *f_networking  = fopen("networking.log",         "w");
    FILE *f_auth        = fopen("authentication.log",     "w");
    FILE *f_mount_fs    = fopen("mount_fs.log",           "w");

    if (!f_startup || !f_failed || !f_warnings || !f_crit_errors ||
        !f_hw_driver || !f_networking || !f_auth || !f_mount_fs) {
        perror("Error opening one of the output files");
        /* Close any that succeeded */
        fclose(fin);
        if (f_startup)     fclose(f_startup);
        if (f_failed)      fclose(f_failed);
        if (f_warnings)    fclose(f_warnings);
        if (f_crit_errors) fclose(f_crit_errors);
        if (f_hw_driver)   fclose(f_hw_driver);
        if (f_networking)  fclose(f_networking);
        if (f_auth)        fclose(f_auth);
        if (f_mount_fs)    fclose(f_mount_fs);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fin) != NULL) {
        /* Call each parser; a single line may match multiple categories */
        parse_startup_timing(line, f_startup);
        parse_failed_services(line, f_failed);
        parse_warnings(line, f_warnings);
        parse_critical_errors(line, f_crit_errors);
        parse_hardware_driver(line, f_hw_driver);
        parse_networking(line, f_networking);
        parse_authentication(line, f_auth);
        parse_mount_fs(line, f_mount_fs);
    }

    /* Close all input/output files */
    fclose(fin);
    fclose(f_startup);
    fclose(f_failed);
    fclose(f_warnings);
    fclose(f_crit_errors);
    fclose(f_hw_driver);
    fclose(f_networking);
    fclose(f_auth);
    fclose(f_mount_fs);

    /* Record end time and compute elapsed */
    double end_time   = omp_get_wtime();
    double elapsed    = end_time - start_time;

    /* Write elapsed time (in seconds) to time_taken.txt (create/overwrite) */
    FILE *ftime = fopen("time_taken.txt", "w");
    if (!ftime) {
        perror("Error opening time_taken.txt");
        return EXIT_FAILURE;
    }
    fprintf(ftime, "Elapsed time: %.6f seconds\n", elapsed);
    fclose(ftime);

    return EXIT_SUCCESS;
}
