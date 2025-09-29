#include <stdio.h>
#include <stdint.h>
#include <string.h> //strncpy
#include <stdlib.h> //atoi
#include "params.h" //CryptoBytes
#include "sign.h"  //crypto_sign
#include "packing.h"
#include "../timeMeasure.h"

int main(int argc, char *argv[]) {
    
    //Suffix als Parameter entgegen nehmen
    if (argc != 3) {
        printf("<Suffix><Iterations>\n");
        return 1;
    }
    char fileNameSuffix[200];
    strncpy(fileNameSuffix, argv[1], sizeof(fileNameSuffix) - 1);
    fileNameSuffix[sizeof(fileNameSuffix) - 1] = '\0';
    int iterations = atoi(argv[2]);

    //Reservierung des Platzes für die Laufzeitmessungen
    Measurement *measurements = malloc(iterations * sizeof(Measurement)); 
    if (measurements == NULL) {
        fprintf(stderr, "Fehler: Speicher konnte nicht allokiert werden\n");
        exit(EXIT_FAILURE);
    }
    char runTimeFilename[512];
    sprintf(runTimeFilename, "%s_GEN_laufzeiten.csv", fileNameSuffix);

    for (int i = 0; i < iterations; i++)
    {

        struct timespec begin = timeMeasure();

        // Schlüsselpaar generieren
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];
        crypto_sign_keypair(pk, sk);

        //Dateinamen erstellen
        char pubkey_filename[200];
        sprintf(pubkey_filename, "%s-public_key.pem", fileNameSuffix);
        char privKey_filename[200];
        sprintf(privKey_filename, "%s-private_key.pem", fileNameSuffix);

        // Öffentlichen Schlüssel in einer .pem-Datei speichern

        FILE *public_key_file = fopen(pubkey_filename, "wb");
        if (!public_key_file) {
            printf("Fehler beim Öffnen der Datei für den öffentlichen Schlüssel.\n");
            return 1;
        }
        for (int i = 0; i < CRYPTO_PUBLICKEYBYTES; i++) {
            fprintf(public_key_file, "%02X", pk[i]);
        }
        fclose(public_key_file);

        // Privaten Schlüssel in einer .pem-Datei speichern

        FILE *private_key_file = fopen(privKey_filename, "wb");
        if (!private_key_file) {
            printf("Fehler beim Öffnen der Datei für den öffentlichen Schlüssel.\n");
            return 1;
        }
        for (int i = 0; i < CRYPTO_SECRETKEYBYTES; i++) {
            fprintf(private_key_file, "%02X", sk[i]);
        }
        fclose(private_key_file);

        struct timespec end = timeMeasure();
        struct timespec diff = timespecDiff(begin, end);
        saveMeasurement(measurements, i, i, diff);


        printf("[GEN %i] Schlüsselpaar wurde erfolgreich generiert.\n",i);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);
    
    return 0;
}
