#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <string.h>

#define ALPHABET_SIZE 26
#define DEFAULT_N 10000000  // Anzahl der Samples

/* --- xorshift128plus --- */
static uint64_t s[2];
// Asigned einen Array mit zwei Einträge, die 64 Bit unasigned
// Static macht, dass die Variable behählt ihren Wert über das Ganze Programm hinweg, egal welche Funktionen benutzt werden
// Dieser Vektor ist dann der Seed des Zufallszahlengenerator

// Hier wird dieser Definiert:
void seed_xorshift128plus(uint64_t seed1, uint64_t seed2) {
    s[0] = seed1;
    s[1] = seed2;
}

// Dies ist eine Funktion, die dann die Bit shift ausführt
uint64_t xorshift128plus() {
    uint64_t s1 = s[0];
    const uint64_t s0 = s[1];
    s[0] = s0;
    s1 ^= s1 << 23;
    s1 ^= s1 >> 17;
    s1 ^= s0 ^ (s0 >> 26);
    s[1] = s1;
    return s[0] + s[1];
}

// Hier wird, dann r zu einer Zahl zwischen 0 und 1 gemacht.
double xorshift128plus_uniform() {
    uint64_t r = xorshift128plus();
    /* Nutzt die oberen 53 Bit für eine double-Zahl in [0,1) */
    return (r >> 11) * (1.0 / (double)(1ULL << 53));
}

// Hier werden uniforme ZV erzeugt mittels der standard rand() funktion
/* --- Standard rand() --- */
double rand_uniform() {
    return ((double)rand() + 0.5) / ((double)RAND_MAX + 1.0);
}

/* --- Qualitätstests (sequentiell) --- 
    Es werden beide Tests in einem Durchlauf berechnet:
    - CAT-Test: Bestimme die Positionen, an denen das Wort (3,1,20) auftritt,
      berechne die Lücken zwischen aufeinanderfolgenden Vorkommen und den Mittelwert.
    - \chi^2-Test für Uniformität: Unterteile [0,1) in k Bins und berechne die Teststatistik.
    Ausgabe: Gesamtzeit, durchschnittliche Lücke (cat_mean_gap),
             Anzahl Vorkommen (num_occurrences) und \chi^2-Wert (chi_sq_uniform).  */
void simulate_quality_tests(int N, double (*rng_func)(),
                            double *time_elapsed, double *cat_mean_gap,
                            int *num_occurrences, double *chi_sq_uniform) {
    int k = 10; // Anzahl der Bins für den \chi^2-Test
    double expected_bin = (double)N / k;
    int bins[10] = {0};
    // Macht einen Array von Länge 10 mit Einträgen 0.

    double *u = (double*) malloc(N * sizeof(double)); // Das ist unser Zufallsvektor initiiert
    int *occurrences = (int*) malloc(N * sizeof(int)); // Hier kommen die tripel hin, die Treffer sind für den CAT test sind.

    
    // Erzeuge N Zufallszahlen
    for (int i = 0; i < N; i++) {
        u[i] = rng_func();
    }
    
    // \chi^2-Test: Zähle, in welches Bin jeder u-Wert fällt
    for (int i = 0; i < N; i++) {
        int bin = (int)(u[i] * k);
        if (bin >= k) bin = k - 1;
        bins[bin]++;
    }
    double chi2 = 0.0;
    for (int i = 0; i < k; i++) {
        double diff = bins[i] - expected_bin;
        chi2 += diff * diff / expected_bin;
    }
    
    // CAT-Test: Berechne I_n = ceil(26*u_n) und suche das Muster (3,1,20)
    int count = 0;
    for (int i = 0; i < N - 2; i++) {
        int I1 = (int)ceil(26.0 * u[i]);
        int I2 = (int)ceil(26.0 * u[i+1]);
        int I3 = (int)ceil(26.0 * u[i+2]);
        if (I1 == 3 && I2 == 1 && I3 == 20) {
            occurrences[count++] = i;
        }
    }
    double avg_gap = 0.0;
    if (count > 1) {
        double sum_gap = 0.0;
        for (int i = 1; i < count; i++) {
            sum_gap += (occurrences[i] - occurrences[i-1]);
        }
        avg_gap = sum_gap / (count - 1);
    }
    
    
    *cat_mean_gap = avg_gap;
    *num_occurrences = count;
    *chi_sq_uniform = chi2;

    free(u);
    free(occurrences);
}

/* --- Leistungssimulation ---
    Misst die Zeit, um N Zufallszahlen zu erzeugen (mit oder ohne Parallelisierung).
    Ein "dummy" Summenwert verhindert Optimierung vom Compiler aus.
    Denn Der Compiler versucht die Laufzeit zu optimieren und würde ohne Dummyvariable
    die alten resultate wieder weiterverwenden. */
double simulate_performance(int N, double (*rng_func)(), const char* schedule) {
    volatile double sum = 0.0;  // verhindert Optimierung
    double start_time, end_time;
    int i;

    if (strcmp(schedule, "seq") == 0) {
        start_time = omp_get_wtime();
        for (i = 0; i < N; i++) {
            sum += rng_func();
        }
        end_time = omp_get_wtime();
    } else {
        start_time = omp_get_wtime();
        // Schedule ist hieru unsere Variable, die aus den Unterschiedlichen Paralelisierungstypen besteht
        // reduction(+:sum) iniiert eine Variable sum=0, die dann lokal für jeden Thread zur Berechnung verwendet wird
        // Am ende werden alle lokalen sum Werte aufsummiert.
        #pragma omp parallel for schedule(runtime) reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += rng_func();
        }
        end_time = omp_get_wtime();
    }
    return end_time - start_time;
}

int main(int argc, char *argv[]) {
    int N = DEFAULT_N;
    // Hier wird nachgeschaut, ob beim Terminal eine Argument/Zahl eingegeben wurde, die für die Anzahl
    // der Zufallsvariablen benutzt wird.
    // Dies kann man in das Terminal eingeben via: ./Nr1 5000000
    if (argc > 1) {
        N = atoi(argv[1]);
    }

    /* RNGs initialisieren */
    srand(time(NULL));
    seed_xorshift128plus((uint64_t) time(NULL), (uint64_t)(time(NULL) ^ 0x5DEECE66DULL));
    // Hier ist (uint64_t) time(NULL) die aktuelle Zeit und wird für den ersten Seed benutzt
    // (uint64_t)(time(NULL) ^ 0x5DEECE66DULL): Berechnet den XOR von der aktuelle Zeit und und der Heximalen Zahl 0x5DEECE66DULL.

    /* Dateien öffnen */
    FILE *fqual = fopen("quality.csv", "w");
    FILE *fperf = fopen("performance.csv", "w");
    // Das if prüft, ob eine der beiden Dateien nicht geöffnet wurde, d.h. den Wert NULL übergibt.
    if (!fqual || !fperf) {
        fprintf(stderr, "Fehler beim Öffnen der Ausgabedateien\n");
        return 1;
    }
    // quality.csv: Für jeden RNG (nur sequenziell)
    fprintf(fqual, "RNG,N,Time,CAT_mean_gap,num_occurrences,ChiSqUniform\n");
    // performance.csv: Für beide RNGs und alle Scheduling-Varianten
    fprintf(fperf, "RNG,Schedule,N,Time\n");

    double time_elapsed, cat_mean_gap, chi_sq_uniform;
    int num_occurrences;

    /* --- Qualitätstests (nur sequenziell) --- */
    simulate_quality_tests(N, rand_uniform, &time_elapsed, &cat_mean_gap, &num_occurrences, &chi_sq_uniform);
    fprintf(fqual, "rand,%d,%.6f,%.6f,%d,%.6f\n", N, time_elapsed, cat_mean_gap, num_occurrences, chi_sq_uniform);
    
    simulate_quality_tests(N, xorshift128plus_uniform, &time_elapsed, &cat_mean_gap, &num_occurrences, &chi_sq_uniform);
    fprintf(fqual, "xorshift128plus,%d,%.6f,%.6f,%d,%.6f\n", N, time_elapsed, cat_mean_gap, num_occurrences, chi_sq_uniform);

    /* --- Leistungstests --- */
    const char* schedules[4] = {"seq", "static", "dynamic", "guided"};
    const char* rng_names[2] = {"rand", "xorshift128plus"};
    double t;
    for (int r = 0; r < 2; r++) {
        double (*rng_func)();
        // strcmp kann zwei Strings vergleichen und gibt 0, wenn diese gleich sind.
        if (strcmp(rng_names[r], "rand") == 0)
            rng_func = rand_uniform;
        else
            rng_func = xorshift128plus_uniform;
        for (int s = 0; s < 4; s++) {
            if (strcmp(schedules[s], "seq") != 0) {
                // Erst wird Geprüft, ob die Bedingung schedules[s], "static") == 0  wahr ist. Sofern ja, dann wird static benutzt
                // Wenn es nicht gilt wird Geprüft, ob strcmp(schedules[s], "dynamic") == 0 gilt. Wenn ja, dann wird dynamic verwendet.
                // Wenn das auch nicht gilt, dann wird guided ausgewählt.
                omp_set_schedule(strcmp(schedules[s], "static") == 0 ? omp_sched_static :
                                 strcmp(schedules[s], "dynamic") == 0 ? omp_sched_dynamic :
                                 omp_sched_guided, 0);
            }
            t = simulate_performance(N, rng_func, schedules[s]);
            fprintf(fperf, "%s,%s,%d,%.6f\n", rng_names[r], schedules[s], N, t);
        }
    }
    fclose(fqual);
    fclose(fperf);
    return 0;
}

// brew install llvm libomp
// clang -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -O2 TestsAndSpeedComparison.c -o TestsAndSpeedComparison
// gcc-14 -fopenmp -O2 TestsAndSpeedComparison.c -o TestsAndSpeedComparison
// ./TestsAndSpeedComparison
