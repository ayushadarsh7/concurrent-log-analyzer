#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // for strcasestr()
#include <omp.h>       // for omp_get_wtime(), omp_set_num_threads()

#define MAX_LINE_LEN 8192
#define NTHREADS 8

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
    /* First, ensure "boot.log" exists and is readable */
    FILE *fin_check = fopen("../boot.log", "r");
    if (!fin_check) {
        perror("Error: could not open boot.log");
        return EXIT_FAILURE;
    }
    fclose(fin_check);

    /* Ensure OpenMP uses exactly NTHREADS threads */
    omp_set_num_threads(NTHREADS);

    /* Record start time */
    double start_time = omp_get_wtime();

    /* Array of parser function pointers, one per thread */
    void (*parsers[NTHREADS])(const char *, FILE *) = {
        parse_startup_timing,
        parse_failed_services,
        parse_warnings,
        parse_critical_errors,
        parse_hardware_driver,
        parse_networking,
        parse_authentication,
        parse_mount_fs
    };

    /* Corresponding output filenames */
    const char *out_filenames[NTHREADS] = {
        "startup_timing.log",
        "failed_services.log",
        "warnings.log",
        "critical_errors.log",
        "hardware_driver.log",
        "networking.log",
        "authentication.log",
        "mount_fs.log"
    };

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        /* Each thread opens "boot.log" for reading */
        FILE *fin = fopen("../boot.log", "r");
        if (!fin) {
            #pragma omp critical
            {
                fprintf(stderr, "Thread %d: Error opening boot.log\n", tid);
            }
            exit(EXIT_FAILURE);
        }

        /* Each thread opens its own output file */
        FILE *fout = fopen(out_filenames[tid], "w");
        if (!fout) {
            #pragma omp critical
            {
                fprintf(stderr, "Thread %d: Error opening %s\n", tid, out_filenames[tid]);
            }
            fclose(fin);
            exit(EXIT_FAILURE);
        }

        char line[MAX_LINE_LEN];
        while (fgets(line, sizeof(line), fin) != NULL) {
            /* Each thread applies its dedicated parser function */
            parsers[tid](line, fout);
        }

        fclose(fin);
        fclose(fout);
    }

    /* Record end time and compute elapsed */
    double end_time = omp_get_wtime();
    double elapsed  = end_time - start_time;

    /* Write elapsed time to time_taken.txt */
    FILE *ftime = fopen("time_taken.txt", "w");
    if (!ftime) {
        perror("Error opening time_taken.txt");
        return EXIT_FAILURE;
    }
    fprintf(ftime, "Elapsed time (parallel): %.6f seconds\n", elapsed);
    fclose(ftime);

    return EXIT_SUCCESS;
}
