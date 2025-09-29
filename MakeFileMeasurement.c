#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //für excelp, sleep und fork
#include "timeMeasure.h"

char measurementPrefix[512];
char runTimeFilename[512];
char fileNameToMeasure[512];

char* getCurrentTime() {
    //Aktuelle Zeit abrufen
    time_t rawtime;
    struct tm *timeinfo;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    rawtime = tv.tv_sec;
    timeinfo = localtime(&rawtime);

    //Zeitformat festlegen
    char *buffer = (char*)malloc(80 * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Fehler: Speicher konnte nicht allokiert werden\n");
        exit(EXIT_FAILURE);
    }
    strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", timeinfo);
    return buffer;
}

void generatePrefix() {
    char *time0 = getCurrentTime();
    sprintf(measurementPrefix, "%s_Makefile",time0);
    free(time0); //Speicher der Zeit-Variablen wieder freigeben
}

void startEnergyMeasurement() {
        //Erstellung der Pfad-Datei:
            char path[256]; // Größe des Pfads 
            snprintf(path, sizeof(path), "%s_messung",measurementPrefix); // %03d stellt sicher, dass die Zahl dreistellig ist (mit führenden Nullen)
            //Konkatinierung des Shell-Befehls:
            char command[200]; // Größe des Befehls anpassen
            snprintf(command, sizeof(command), "gtimeout 600s sudo powermetrics -i 5000 --show-all -o \"%s\"", path);

            kill(getppid(), SIGUSR1);
            printf("+++EnergieMessung wird gestartet+++\n"); //Debugging
            system(command);
}

int main(int argc, char *argv[]) {
            
    generatePrefix();
    printf("+++ Name dieser Messung: %s +++\n", measurementPrefix); //Debugging  
    
    //Reservierung des Platzes für die Laufzeitmessungen
        char runTimeFilename[512];
        sprintf(runTimeFilename, "%s_laufzeiten.csv", measurementPrefix);
               
        pid_t pid = fork(); //Prozess zum Starten der Messung erstellen
            if (pid < 0) { //Überprüfe, ob die fork() erfolgreich war

                fprintf(stderr, "Fehler bei der Erstellung des Kindprozesses\n");
                exit(EXIT_FAILURE);

            } else if (pid == 0) { //Dieser Code wird im Kindprozess ausgeführt

                startEnergyMeasurement();
                exit(EXIT_FAILURE);

            } else { // Dieser Code wird im Elternprozess ausgeführt

                    //Warten auf Start der Energiemessung durch den Kindprozess
                        sigset_t set;
                        sigemptyset(&set);
                        sigaddset(&set, SIGUSR1);
                        // Warten auf das SIGUSR1-Signal
                        int sig;
                        sigwait(&set, &sig);

                    sleep(1);
                    //Messung der Zeit vor Ausführung
                        struct timespec begin = timeMeasure();
                    
                    //Start des Makefiles
                        system("make");

                    //Messung der Zeit nach Ausführung
                        struct timespec end = timeMeasure();
                    
                    //Berechnung und Speicherung der Zeitdifferenz 
                        struct timespec diff = timespecDiff(begin, end); //Differenz in Nanosekunden
                        long seconds = diff.tv_sec;
                        long milliseconds = (long)(diff.tv_nsec / 1.0e6);
                        long microseconds = (long)((diff.tv_nsec % 1000000) / 1.0e3);
                        long nanoseconds = diff.tv_nsec % 1000;
                        FILE *file = fopen(runTimeFilename, "w");
                            if (file == NULL) {
                                fprintf(stderr, "Fehler: Datei %s konnte nicht zum Schreiben geöffnet werden\n", runTimeFilename);
                                exit(EXIT_FAILURE);
                            }
                            // Schreibe die Werte der Variablen in die Datei
                            fprintf(file, "Seconds: %ld\n", seconds);
                            fprintf(file, "Milliseconds: %ld\n", milliseconds);
                            fprintf(file, "Microseconds: %ld\n", microseconds);
                            fprintf(file, "Nanoseconds: %ld\n", nanoseconds);
                        fclose(file);

                //Manuelle Beendung der Energiemessung
                    sleep(3);
                    system("pkill -9 -x powermetrics");
                    //Warte auf den Kindprozess
                    int status;
                    waitpid(pid, &status, 0);
                    printf("+++EnergieMessung beendet.+++\n"); //Debugging
            }
    return 0;
}
