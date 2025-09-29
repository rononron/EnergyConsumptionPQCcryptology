#ifndef timeMeasure_H
#define timeMeasure_H

#include <time.h>

typedef struct {
    int iteration;
    long seconds;
    long milliseconds;
    long microseconds;
    long nanoseconds;
} Measurement;

struct timespec timeMeasure(void);
struct timespec timespecDiff(struct timespec start, struct timespec end);
void saveMeasurement(Measurement *measurements, int index, int iteration, struct timespec diff);
void writeMeasurementsToCSV(const char *filename, Measurement *measurements, int count);
void freeMeasurements(Measurement *measurements);

#endif
