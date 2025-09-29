#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/obj_mac.h> //Brainpool
#include "../timeMeasure.h"

int main(int argc, char *argv[]) {
    
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

    EC_KEY *eckey = EC_KEY_new_by_curve_name(NID_secp384r1);
    //EC_KEY *eckey = EC_KEY_new_by_curve_name(NID_brainpoolP384r1);

        if (!eckey) {
        fprintf(stderr, "Fehler beim Erstellen des Schlüssels.\n");
        return 1;
    }

    if (!EC_KEY_generate_key(eckey)) {
        fprintf(stderr, "Fehler beim Generieren des Schlüssels.\n");
        return 1;
    }

    // Öffentlichen Schlüssel speichern
        char pubkey_filename[200];
        sprintf(pubkey_filename, "%s-public_key.pem", fileNameSuffix);
        FILE *pubkey_file = fopen(pubkey_filename, "wb");
        if (!pubkey_file) {
            perror("Fehler beim Öffnen der Datei für den öffentlichen Schlüssel");
            return 1;
        }
        if (!PEM_write_EC_PUBKEY(pubkey_file, eckey)) {
            fprintf(stderr, "Fehler beim Speichern des öffentlichen Schlüssels.\n");
            return 1;
        }
        fclose(pubkey_file);

    // Privaten Schlüssel speichern
        char privkey_filename[200];
        sprintf(privkey_filename, "%s-private_key.pem", fileNameSuffix);
        FILE *privkey_file = fopen(privkey_filename, "wb");
        if (!privkey_file) {
            perror("Fehler beim Öffnen der Datei für den privaten Schlüssel");
            return 1;
        }
        if (!PEM_write_ECPrivateKey(privkey_file, eckey, NULL, NULL, 0, NULL, NULL)) {
            fprintf(stderr, "Fehler beim Speichern des privaten Schlüssels.\n");
            return 1;
        }
        fclose(privkey_file);
        EC_KEY_free(eckey);

        struct timespec end = timeMeasure();
        struct timespec diff = timespecDiff(begin, end);
        saveMeasurement(measurements, i, i, diff);


        printf("[GEN %i] Schlüsselpaar wurde erfolgreich generiert.\n",i);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);
    return 0;
}
