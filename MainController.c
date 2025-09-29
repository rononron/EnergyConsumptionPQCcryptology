#include <stdio.h>
#include <stdlib.h> //atoi
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //für excelp, sleep und fork

char measurementPrefix[512]; //Benennung der Messung
char runTimeFilename[512]; //Name der Laufzeitmessdatei
char stepString[4]; //Aktueller Schritt (GEN, SIG oder VER)
char messageFileName[512]; //Name der Datei der zufälligen Nachricht

//Struktur zur Speicherung der Einzel-Laufzeiten
typedef struct {
    int iteration;
    char step[4];
    long seconds;
    long milliseconds;
    long microseconds;
    long nanoseconds;
} Measurement;

//Funktion zur Ermittlung der aktuellen Zeit
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

//Funktion zur Messung der Zeit seit Systemstart in Nanosekunden, ohne Einfluss durch Protokolle wie NTP
struct timespec timeMeasure() {
    struct timespec time; 
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    return time;
}

// Funktion zur Berechnung der Differenz zwischen zwei timespec-Strukturen
struct timespec timespecDiff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        //Diese Abfrage dient der Berechnung der korrekten Differenz wenn die Endzeit größer der Startzeit
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

//Funktion zum speichern jeder Zeitmessung in der Strukturvariablen
void saveMeasurement(Measurement *measurements, int index, int iteration, struct timespec diff) {
    //Umwandlung
        long seconds = diff.tv_sec;
        long milliseconds = (long)(diff.tv_nsec / 1.0e6);
        long microseconds = (long)((diff.tv_nsec % 1000000) / 1.0e3);
        long nanoseconds = diff.tv_nsec % 1000;
    //Speicherung
        measurements[index].iteration = iteration;
        strcpy(measurements[index].step, stepString);
        measurements[index].seconds = seconds;
        measurements[index].milliseconds = milliseconds;
        measurements[index].microseconds = microseconds;
        measurements[index].nanoseconds = nanoseconds;
}

// Funktion zum Schreiben der Messdaten in die CSV-Datei
void writeMeasurementsToCSV(const char *filename, Measurement *measurements, int count) {
    sprintf(runTimeFilename, "%s_laufzeiten.csv", measurementPrefix);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Fehler beim Öffnen der Datei.\n");
        return;
    }
    // Header schreiben
    fprintf(file, "Iteration,STEP,SEC,MLS,MCS,NAS\n");
    // Messdaten schreiben
    for (int i = 1; i < (count*3+1); i++) {
        fprintf(file, "%d,%s,%03ld,%03ld,%03ld,%03ld\n",
                measurements[i].iteration,
                measurements[i].step,
                measurements[i].seconds,
                measurements[i].milliseconds,
                measurements[i].microseconds,
                measurements[i].nanoseconds);
    }
    fclose(file);
}

//Funktion zur Freigabe des Speichers der Struktur MEasurement, welche bis zu 30.000 Einträge haben kann
void freeMeasurements(Measurement *measurements) {
    if (measurements != NULL) {
        free(measurements);
    }
}

//Funktion zur Erstellung des Präfix aller Ergebnisdateien 
void generatePrefix(char *algorithm, int iterations) {
    char *time0 = getCurrentTime();
    sprintf(measurementPrefix, "./Logs/%s_%s_%i",time0, algorithm, iterations);
    free(time0); //Speicher der Zeit-Variablen wieder freigeben
}

//Funktion zum Start der Energiemessung mit powermetrics
void startEnergyMeasurement(int iterations) {
    //Erstellung der Pfad-Datei:
    char path[256];
    snprintf(path, sizeof(path), "%s_%s_messung",measurementPrefix, stepString);
    //Konkatinierung des Shell-Befehls:
        char command[200];
        snprintf(command, sizeof(command), "sudo powermetrics -i 5000 --show-all -o \"%s\"", path);

    kill(getppid(), SIGUSR1);
    printf("+++EnergieMessung wird gestartet+++\n"); //Debugging
    system(command);
}

// Funktion zum Generieren einer zufälligen Zeichenfolge in Bytes
void generateRandomString(int length) {
    // Initialisieren des Zufallszahlengenerators mit der aktuellen Zeit
        srand(time(NULL));
    // Generieren der zufälligen Zeichenfolge
        char message[length+1]; //+1 für den Nulloperator
        for (int i = 0; i < length; i++) {
            // Generieren eines zufälligen Zeichens (ASCII-Werte von 40 bis 126 repräsentieren druckbare Zeichen)
            char randomChar = rand() % (126 - 40 + 1) + 40;
            message[i] = randomChar;
        }
        message[0] = '"';
        message[length - 1] = '"';
        message[length] = '\0';

    // Generieren des Dateinamens
        sprintf(messageFileName, "%s-message.txt", measurementPrefix);
    // Öffnen der Datei zum Schreiben
        FILE *file = fopen(messageFileName, "w");
        if (file == NULL) {
            perror("Fehler beim Öffnen der Datei");
            return;
        }
    // Schreiben der Nachricht in die Datei
        fprintf(file, "%s\n", message);
    // Schließen der Datei
        fclose(file);
}

//Funktion zum Definieren des aktuellen Schritts als char-Variable
void createStepString(int step) {
    switch (step) {
        case 1:
            strcpy(stepString, "GEN");
            break;
        case 2:
            strcpy(stepString, "SIG");
            break;
        case 3:
            strcpy(stepString, "VER");
            break;
        default:
            strcpy(stepString, "ERR"); // Fehlerbehandlung für unbekannte Schritte
            break;
    }
}

//Funktion zum Löschen temporärer Dateien
void deleteTempFiles() {
    // Pfade zu den temporären Dateien
        char privatekeyFileName[512];
            sprintf(privatekeyFileName, "%s-private_key.pem", measurementPrefix);
        char publicKeyFileName[512];
            sprintf(publicKeyFileName, "%s-public_key.pem", measurementPrefix);
        char signatureFileName[512];
            sprintf(signatureFileName, "%s-signature.bin", measurementPrefix);
        char *filePaths[] = {
            messageFileName, // Pfad zur msg-Datei
            signatureFileName, // Pfad zur sig-Datei
            publicKeyFileName, // Pfad zur pub-Datei
            privatekeyFileName // Pfad zur pri-Datei
        };
    // Löschen der Dateien
        for (int i = 0; i < 4; i++) {
            if (unlink(filePaths[i]) == 0) {
                //printf("Datei %s erfolgreich gelöscht.\n", filePaths[i]);
            } else {
                printf("Fehler beim Löschen der Datei %s.\n", filePaths[i]);
            }
        }
        //printf("DAteien gelöscht\n");
}

int main(int argc, char *argv[]) {
    //Bestimmungung ob alle Argumente korrekt übergeben wurden
        if(argc != 4) {
            printf(" <Iterations> <MessageLength> <Algorithmus> \n");
            return 1;
        }
    
    //Speicherung der Eingabeparameter
        int iterations = atoi(argv[1]);
            Measurement *measurements = malloc(3 * sizeof(Measurement)); //Reservierung des Platzes für die Laufzeitmessungen
            //3 --> GEN SIG VER
            if (measurements == NULL) {
                fprintf(stderr, "Fehler: Speicher konnte nicht allokiert werden\n");
                exit(EXIT_FAILURE);
            }
        
        char algorithm[64];
            if(atoi(argv[3]) == 1) { //TEST
                sprintf(algorithm, "CPUTEST");
            } else if(atoi(argv[3]) == 2) {
                sprintf(algorithm, "ECDSA");
            } else if(atoi(argv[3]) == 3) {
                sprintf(algorithm, "dilithium3");
            } else if(atoi(argv[3]) == 4) {
                sprintf(algorithm, "sphincsplus3");
            }

    //Benennung der Messung = Generierung des Präfix
        generatePrefix(algorithm, iterations);
        printf("+++ Name dieser Messung: %s +++\n", measurementPrefix);
        int messageLength = atoi(argv[2]);
            generateRandomString(messageLength);
  
        for(int j = 1; j < 4;j++) {
            
            createStepString(j);
            
            pid_t pid = fork(); //Prozess zum Starten der Messung erstellen
            if (pid < 0) { //Überprüung, ob die fork() erfolgreich war

                fprintf(stderr, "Fehler bei der Erstellung des Kindprozesses\n");
                exit(EXIT_FAILURE);

            } else if (pid == 0) { //Dieser Code wird im Kindprozess ausgeführt

                //startEnergyMeasurement(iterations);
                exit(EXIT_FAILURE);

            } else { // Dieser Code wird im Elternprozess ausgeführt

                    //Warten auf Start der Energiemessung durch den Kindprozess
                        sigset_t set;
                        sigemptyset(&set);
                        sigaddset(&set, SIGUSR1);
                        // Warten auf das SIGUSR1-Signal
                        int sig;
                        sigwait(&set, &sig);

                    sleep(3);
                    //Messung der Zeit vor Ausführung
                        struct timespec begin = timeMeasure();
                    
                    //Start des zu Messenden Prozesses
                        char algorithmCommand[256];                       
                        if(j == 1){
                           snprintf(algorithmCommand, sizeof(algorithmCommand), "./%s%s %s %i",algorithm, "/GEN", measurementPrefix, iterations);
                           printf("[]: %s KEYGENERATION\n", algorithm); //Debugging
                        } else if (j == 2){
                            snprintf(algorithmCommand, sizeof(algorithmCommand), "./%s%s %s %i",algorithm, "/SIG", measurementPrefix, iterations);
                            printf("[]: %s SIGNATURE\n", algorithm); //Debugging
                        } else if (j == 3) {
                            snprintf(algorithmCommand, sizeof(algorithmCommand), "./%s%s %s %i",algorithm, "/VER", measurementPrefix, iterations);
                            printf("[]: %s VERIFICATION\n", algorithm); //Debugging
                        }
                        system(algorithmCommand);

                    //Messung der Zeit nach Ausführung
                        struct timespec end = timeMeasure();
                    
                    //Berechnung der Zeitdifferenz 
                        struct timespec diff = timespecDiff(begin, end); //Differenz in Nanosekunden

                    //Messdauer in Struktur speichern
                        saveMeasurement(measurements, j,j, diff);

                //Manuelle Beendung der Energiemessung
                    sleep(3);
                   // system("pkill -9 -x powermetrics");
                    //Warte auf den Kindprozess
                    int status;
                    waitpid(pid, &status, 0);
                    printf("+++EnergieMessung beendet.+++\n"); //Debugging

            //Auswertung der Messergebnisse via skript
                //Erstellung des Shell-Befehls:
                    char commandCalc[200];
                    snprintf(commandCalc, sizeof(commandCalc), "sudo ./auswertung.sh \"%s\" \"%s\"", measurementPrefix, stepString);
                if (atoi(argv[3]) != 1 && iterations > 399) //Nicht-CPU-Test-Programme sollen bei weniger als 400 Iteration nicht ausgewertet werden
                {
                system(commandCalc);
                }
                else if (atoi(argv[3]) == 1) //CPU-Test soll immer ausgewertet werden
                {
                system(commandCalc);
                }
            }
        }
    //Schreiben der Laufzeiten in die Datei
        writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
        freeMeasurements(measurements);
    if(atoi(argv[3]) !=1) //Keine Temps zu löschen bei CPU-TEST
    { 
        //deleteTempFiles();
    }
    return 0;
}