#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_LOOP_ITERATIONS 1000000000

int main() {


    // Zeitpunkt des Programmstarts
    time_t start_time = time(NULL);

    // Berechnungsschleife, die die CPU stark beansprucht
    while(1) {
        // Zeitdifferenz seit dem Programmstart
        time_t current_time = time(NULL);
        double time_difference = difftime(current_time, start_time);

        // Sicherheitsausgang nach 1 Sekunden
        if (time_difference >= 30) {
            printf("Sicherheitsausgang nach 30 Sekunde.\n");
            break;
        }

        // Dummy-Berechnung, der die CPU belastet
        int result = 0;
        for(int i = 0; i < MAX_LOOP_ITERATIONS; ++i) {
            result += i;
        }

        // Verzögerung, um die CPU-Auslastung zu begrenzen
        for (int i = 0; i < MAX_LOOP_ITERATIONS; ++i) {
            result *= i;
        }
    }

    return 0;
}
