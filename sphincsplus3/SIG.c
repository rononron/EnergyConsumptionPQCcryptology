#include <stdio.h>
#include <stdlib.h> //atoi
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "rng.h"
#include "api.h"
#include "params.h"
#include "../timeMeasure.h"

uint8_t sk[CRYPTO_SECRETKEYBYTES];

void loadSK(char *privatekeyFileName) {

    // Privaten Schlüssel laden
    FILE *private_key_file = fopen(privatekeyFileName, "r");
    if (!private_key_file) {
        printf("Fehler beim Öffnen der Datei für den öffentlichen Schlüssel.\n");
        return;
    }

    // Länge des privaten Schlüssels in der .pem-Datei bestimmen
    fseek(private_key_file, 0L, SEEK_END);
    long fileSize = ftell(private_key_file);
    rewind(private_key_file);

    if (fileSize != CRYPTO_SECRETKEYBYTES * 2) { // * 2, weil jedes Byte im hexadezimalen Format zwei Zeichen hat
        printf("Ungültige Länge des öffentlichen Schlüssels in der Datei.\n");
        fclose(private_key_file);
        return;
    }

    for (int i = 0; i < CRYPTO_SECRETKEYBYTES; i++) {
        int byteValue;
        if (fscanf(private_key_file, "%2X", &byteValue) != 1) {
            printf("Fehler beim Lesen des öffentlichen Schlüssels aus der Datei.\n");
            fclose(private_key_file);
            return;
        }
        sk[i] = (uint8_t)byteValue;
    }

    fclose(private_key_file);
}

int main(int argc, char *argv[]) {
   if (argc != 3) {
        printf("<Suffix><Iterations>\n");
        return 1;
    }
    char fileNameSuffix[512];
    strncpy(fileNameSuffix, argv[1], sizeof(fileNameSuffix) - 1);
    fileNameSuffix[sizeof(fileNameSuffix) - 1] = '\0';
    char messageFileName[512];
    sprintf(messageFileName, "%s-message.txt", fileNameSuffix);
    char signatureFileName[512];
    sprintf(signatureFileName, "%s-signature.bin", fileNameSuffix);
    char privatekeyFileName[512];
    sprintf(privatekeyFileName, "%s-private_key.pem", fileNameSuffix);
    
       int iterations = atoi(argv[2]);

    //Reservierung des Platzes für die Laufzeitmessungen
    Measurement *measurements = malloc(iterations * sizeof(Measurement)); 
    if (measurements == NULL) {
        fprintf(stderr, "Fehler: Speicher konnte nicht allokiert werden\n");
        exit(EXIT_FAILURE);
    }
    char runTimeFilename[512];
    sprintf(runTimeFilename, "%s_SIG_laufzeiten.csv", fileNameSuffix);

    for (int i = 0; i < iterations; i++)
    {

        struct timespec begin = timeMeasure();

    // Nachricht zum Signieren laden
    FILE *message_file = fopen(messageFileName, "r");
    if (message_file == NULL) {
        printf("Fehler beim Öffnen der Datei %s.\n", messageFileName);
        return 1;
    }
    fseek(message_file, 0, SEEK_END);
    size_t message_length = ftell(message_file);
    fseek(message_file, 0, SEEK_SET);
    uint8_t *message = malloc(message_length + 1);
    if (message == NULL) {
        printf("Fehler beim Reservieren von Speicher für die Nachricht.\n");
        fclose(message_file);
        return 1;
    }
    fread(message, 1, message_length, message_file);
    message[message_length] = '\0';
    fclose(message_file);

    // Privaten Schlüssel aus den PEM-Dateien laden
        loadSK(privatekeyFileName);

    // Signatur der Nachricht erstellen
    uint8_t signature[CRYPTO_BYTES];
    size_t signature_length;
    if (crypto_sign_signature(signature, &signature_length, (const uint8_t *)message, message_length, sk) != 0) {
        printf("Fehler beim Erstellen der Signatur.\n");
        return 1;
    }

    // Speichern der Signatur in einer Datei
    FILE *signature_file = fopen(signatureFileName, "wb");
    if (signature_file == NULL) {
        printf("Fehler beim Öffnen der Datei für die Signatur.\n");
        return 1;
    }
    fwrite(signature, 1, signature_length, signature_file);
    fclose(signature_file);
    unsigned long long sigLengthTEst = crypto_sign_bytes();

    struct timespec end = timeMeasure();
    struct timespec diff = timespecDiff(begin, end);
    saveMeasurement(measurements, i, i, diff);


        printf("[SIG %i] Signatur erstellt.\n",i);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);

    return 0;
}
