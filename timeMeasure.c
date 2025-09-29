#include "timeMeasure.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> //für free()

//Funktion zur Messung der Zeit in Nanosekunden
struct timespec timeMeasure(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    return time;
}

//Funktion zur Berechnung der Differenz zweiter timespec-Variablen
struct timespec timespecDiff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

//Funktion zum temporären Speichern der Laufzeit in einer Variable
void saveMeasurement(Measurement *measurements, int index, int iteration, struct timespec diff) {
    long seconds = diff.tv_sec;
    long milliseconds = (long)(diff.tv_nsec / 1.0e6);
    long microseconds = (long)((diff.tv_nsec % 1000000) / 1.0e3);
    long nanoseconds = diff.tv_nsec % 1000;

    measurements[index].iteration = iteration;
    measurements[index].seconds = seconds;
    measurements[index].milliseconds = milliseconds;
    measurements[index].microseconds = microseconds;
    measurements[index].nanoseconds = nanoseconds;
}

//Funktion zum Schreiben der Messdaten in eine CSV
void writeMeasurementsToCSV(const char *filename, Measurement *measurements, int count) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Fehler beim Öffnen der Datei.\n");
        return;
    }

    fprintf(file, "Iteration,SEC,MLS,MCS,NAS\n");

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d,%03ld,%03ld,%03ld,%03ld\n",
                measurements[i].iteration,
                measurements[i].seconds,
                measurements[i].milliseconds,
                measurements[i].microseconds,
                measurements[i].nanoseconds);
    }

    fclose(file);
}

//Funktion zur Befreiung des Speichers der Struktur "Measurement"
void freeMeasurements(Measurement *measurements) {
    if (measurements != NULL) {
        free(measurements);
    }
}
