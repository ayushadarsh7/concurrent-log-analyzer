/* 
    ---------------------
         AA
        A  A
       AAAAAA
      A      A
     A        A
    ---------------------
    Created by Ayush Adarsh
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <omp.h>    

#define MAX_LINE    8192   // maximum characters per line
#define OVECCOUNT   30     // enough for up to ~10 capture groups

// Helper: compile one PCRE pattern, or exit on failure
static pcre* compile_regex(const char *pattern)
{
    const char *pcreError;
    int erroffset;
    pcre *re = pcre_compile(pattern, PCRE_CASELESS, &pcreError, &erroffset, NULL);
    if (!re) {
        fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, pcreError);
        exit(EXIT_FAILURE);
    }
    return re;
}

// Structure to hold one category’s info
typedef struct {
    const char *input_filename;   // e.g. "../authentication.log"
    const char *output_filename;  // e.g. "authentication_issues.log"
    const char **regex_patterns;  // NULL-terminated array of regex strings
} Category;

//  1) “authentication.log” → look for failed-login/anomalous authentication
static const char *auth_patterns[] = {
    "(?i)Failed password for",
    "(?i)authentication failure",
    "(?i)Invalid user",
    "(?i)sudo:.*authentication failure",
    "(?i)Root login",
    NULL
};

//  2) “critical_errors.log” → kernel/customer-critical errors
static const char *crit_patterns[] = {
    "(?i)kernel panic",
    "(?i)panic:",
    "(?i)segfault",
    "(?i)BUG: unable to handle",
    "(?i)OOM killer invoked",
    "(?i)call trace",
    "(?i)kernel BUG",
    NULL
};

//  3) “failed_services.log” → service‐start failures
static const char *svc_patterns[] = {
    "(?i)Failed to start",
    "(?i)Dependency failed for",
    "(?i)Start request repeated too quickly",
    "(?i)start-limit-hit",
    "(?i)Timed out",
    "(?i)exited with status [1-9][0-9]*",
    NULL
};

//  4) “hardware_driver.log” → driver/hardware anomalies
static const char *hw_patterns[] = {
    "(?i)driver .* failed to load",
    "(?i)firmware .* failed",
    "(?i)I/O error",
    "(?i)device not found",
    "(?i)timeout.*(usb|pci|sda|nvme)",
    "(?i)PCIe AER",
    "(?i)firmware: failed to load",
    NULL
};

//  5) “mount_fs.log” → filesystem/mount issues
static const char *fs_patterns[] = {
    "(?i)mount: .* failed",
    "(?i)fsck .* error",
    "(?i)read-only file system",
    "(?i)filesystem corruption",
    "(?i)disk full",
    "(?i)I/O error",
    "(?i)no such file or directory",
    NULL
};

//  6) “networking.log” → networking/DHCP/DNS anomalies
static const char *net_patterns[] = {
    "(?i)Link is down",
    "(?i)DHCPDISCOVER.*timeout",
    "(?i)DNS lookup failure",
    "(?i)temporary failure in name resolution",
    "(?i)Network is unreachable",
    "(?i)IP conflict",
    "(?i)duplicate IP",
    "(?i)device eth[0-9]+: no carrier",
    "(?i)interface .* not found",
    "(?i)Link down",
    NULL
};

//  7) “startup_timing.log” → slow/problematic units
static const char *start_patterns[] = {
    "(?i)Start request repeated too quickly",
    "(?i)Timed out",
    "(?i)start-limit-hit",
    "(?i)watchdog",
    "(?i)job [0-9]+ failed",
    NULL
};

//  8) “warnings.log” → general warning‐level messages
static const char *warn_patterns[] = {
    "(?i)deprecated",
    "(?i)low memory",
    "(?i)out of memory",
    "(?i)throttling",
    "(?i)overrun",
    "(?i)ACPI",
    "(?i)thermal warning",
    "(?i)overheat",
    "(?i)buffer overflow",
    "(?i)stack overflow",
    NULL
};

// Assemble all eight categories into an array of Category structs.
// Output filenames live in the current folder (Log_Analysis/).
static Category categories[] = {
    { "../authentication.log",   "authentication_issues.log",   auth_patterns },
    { "../critical_errors.log",  "critical_errors_issues.log",  crit_patterns },
    { "../failed_services.log",  "failed_services_issues.log",  svc_patterns },
    { "../hardware_driver.log",  "hardware_driver_issues.log",  hw_patterns },
    { "../mount_fs.log",         "mount_fs_issues.log",         fs_patterns },
    { "../networking.log",       "networking_issues.log",       net_patterns },
    { "../startup_timing.log",   "startup_issues.log",          start_patterns },
    { "../warnings.log",         "warnings_issues.log",         warn_patterns }
};

int main(void)
{
    double start_time = omp_get_wtime();

    int num_categories = sizeof(categories) / sizeof(categories[0]);
    char line[MAX_LINE];
    int ovector[OVECCOUNT];

    // Parallelize over categories: each thread handles one category index
    #pragma omp parallel for schedule(static)
    for (int c = 0; c < num_categories; ++c) {
        const char *infile    = categories[c].input_filename;
        const char *outfile   = categories[c].output_filename;
        const char **patterns = categories[c].regex_patterns;

        // 1) Open input category file (parent folder)
        FILE *fin = fopen(infile, "r");
        if (!fin) {
            #pragma omp critical
            fprintf(stderr, "Error: Could not open %s for reading\n", infile);
            continue;  // skip this category
        }

        // 2) Open output “issues” file (current folder)
        FILE *fout = fopen(outfile, "w");
        if (!fout) {
            #pragma omp critical
            fprintf(stderr, "Error: Could not open %s for writing\n", outfile);
            fclose(fin);
            continue;
        }

        // 3) Count patterns, compile them
        int pat_count = 0;
        while (patterns[pat_count] != NULL) {
            ++pat_count;
        }

        pcre **compiled = malloc(pat_count * sizeof(pcre *));
        if (!compiled) {
            #pragma omp critical
            perror("malloc");
            fclose(fin);
            fclose(fout);
            continue;
        }
        for (int i = 0; i < pat_count; ++i) {
            compiled[i] = compile_regex(patterns[i]);
        }

        // 4) Read each line, test all regexes
        while (fgets(line, sizeof(line), fin)) {
            for (int i = 0; i < pat_count; ++i) {
                int rc = pcre_exec(
                    compiled[i],      // compiled pattern
                    NULL,             // no extra data
                    line,             // subject string
                    (int)strlen(line),// length of subject
                    0,                // start at offset 0
                    0,                // default options
                    ovector,          // output vector
                    OVECCOUNT         // size of ovector
                );
                if (rc >= 0) {
                    // Match found → write the line once, then break
                    fputs(line, fout);
                    break;
                }
            }
        }

        // 5) Cleanup: free compiled patterns, close files
        for (int i = 0; i < pat_count; ++i) {
            pcre_free(compiled[i]);
        }
        free(compiled);
        fclose(fin);
        fclose(fout);
    }

    double end_time = omp_get_wtime();
    double elapsed  = end_time - start_time;

    // Write elapsed time to time_taken.txt (current folder)
    FILE *ftime = fopen("time_taken.txt", "w");
    if (!ftime) {
        perror("Error opening time_taken.txt");
        return EXIT_FAILURE;
    }
    fprintf(ftime, "Elapsed time: %.6f seconds\n", elapsed);
    fclose(ftime);

    return EXIT_SUCCESS;
}
