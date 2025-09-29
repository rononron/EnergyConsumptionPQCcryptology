#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h> //atoi
#include <ctype.h>
#include "rng.h"
#include "api.h"
#include "params.h"
#include "../timeMeasure.h"

uint8_t pk[CRYPTO_PUBLICKEYBYTES];

void loadPK(char *publicKeyFileName) {

    // Öffentlichen Schlüssel laden
    FILE *publicKeyFile = fopen(publicKeyFileName, "r");
    if (!publicKeyFile) {
        printf("Fehler beim Öffnen der Datei für den öffentlichen Schlüssel.\n");
        return;
    }

    // Länge des öffentlichen Schlüssels in der .pem-Datei bestimmen
    fseek(publicKeyFile, 0L, SEEK_END);
    long fileSize = ftell(publicKeyFile);
    rewind(publicKeyFile);

    if (fileSize != CRYPTO_PUBLICKEYBYTES * 2) { // * 2, weil jedes Byte im hexadezimalen Format zwei Zeichen hat
        printf("Ungültige Länge des öffentlichen Schlüssels in der Datei.\n");
        fclose(publicKeyFile);
        return;
    }

    for (int i = 0; i < CRYPTO_PUBLICKEYBYTES; i++) {
        int byteValue;
        if (fscanf(publicKeyFile, "%2X", &byteValue) != 1) {
            printf("Fehler beim Lesen des öffentlichen Schlüssels aus der Datei.\n");
            fclose(publicKeyFile);
            return;
        }
        pk[i] = (uint8_t)byteValue;
    }

    fclose(publicKeyFile);
}

int main(int argc, char *argv[]) {
    //Parameter laden
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
        char publicKeyFileName[512];
            sprintf(publicKeyFileName, "%s-public_key.pem", fileNameSuffix);

    int iterations = atoi(argv[2]);

    //Reservierung des Platzes für die Laufzeitmessungen
    Measurement *measurements = malloc(iterations * sizeof(Measurement)); 
    if (measurements == NULL) {
        fprintf(stderr, "Fehler: Speicher konnte nicht allokiert werden\n");
        exit(EXIT_FAILURE);
    }
    char runTimeFilename[512];
    sprintf(runTimeFilename, "%s_VER_laufzeiten.csv", fileNameSuffix);

    for (int i = 0; i < iterations; i++)
    {

        struct timespec begin = timeMeasure();

 
    // Nachricht laden
        // Nachricht aus der angegebenen Datei laden
        FILE *messageFile = fopen(messageFileName, "r");
        if (messageFile == NULL) {
            printf("Fehler beim Öffnen der Datei %s.\n", messageFileName);
            return 1;
        }
        // Bestimmen der Länge der Nachricht
        fseek(messageFile, 0, SEEK_END);
        size_t messageLength = ftell(messageFile);
        fseek(messageFile, 0, SEEK_SET);
        // Speicher für die Nachricht reservieren
        char *message = malloc(messageLength + 1); // +1 für das Nullterminierungszeichen
        if (message == NULL) {
            printf("Fehler beim Reservieren von Speicher für die Nachricht.\n");
            fclose(messageFile);
            return 1;
        }
        // Nachricht lesen
        fread(message, 1, messageLength, messageFile);
        message[messageLength] = '\0'; // Nullterminierungszeichen hinzufügen
        fclose(messageFile);
    
    //Signatur laden
    uint8_t signature[CRYPTO_BYTES];
        // Laden der Signatur aus der Datei
        FILE *signatureFile = fopen(signatureFileName, "rb");
            if (signatureFile == NULL) {
                printf("Fehler beim Öffnen der Datei für die geladene Signatur.\n");
                return 1;
        }
        size_t signatureLength = fread(signature, 1, CRYPTO_BYTES, signatureFile);
            if (signatureLength != CRYPTO_BYTES) {
            printf("Fehler beim Lesen der Signatur aus der Datei.\n");
            fclose(signatureFile);
            return 1;
            }
        fclose(signatureFile);
        if (signatureLength != CRYPTO_BYTES) {
            printf("Fehler beim Laden der Signatur. Falsche Länge.\n");
            return 1;
        }

    // Öffentlichen Schlüssel laden
        loadPK(publicKeyFileName);

    // Signatur verifizieren
    int verificationResult = crypto_sign_verify(signature, signatureLength, (uint8_t *)message, messageLength, pk);

    if (verificationResult == 0) {
        printf("[VER %i] Die Signatur ist gültig.\n", i);
    } else {
        printf("[VER %i] Die Signatur ist ungültig.\n", i);
    }

        struct timespec end = timeMeasure();
        struct timespec diff = timespecDiff(begin, end);
        saveMeasurement(measurements, i, i, diff);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);
    return 0;
}
