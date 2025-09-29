#include <stdio.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include "../timeMeasure.h"

int main(int argc, char *argv[]) {
    //Parameter laden
    if (argc != 3) {
        printf("<Suffix><Iterations>\n");
        return 1;
    }
        char fileNameSuffix[200];
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
        // Nachricht aus der angegebenen Datei laden
        FILE *message_file = fopen(messageFileName, "r");
        if (message_file == NULL) {
            printf("Fehler beim Öffnen der Datei %s.\n", messageFileName);
            return 1;
        }
        // Bestimmen der Länge der Nachricht
        fseek(message_file, 0, SEEK_END);
        size_t message_length = ftell(message_file);
        fseek(message_file, 0, SEEK_SET);
        // Speicher für die Nachricht reservieren
        char *message = malloc(message_length + 1); // +1 für das Nullterminierungszeichen
        if (message == NULL) {
            printf("Fehler beim Reservieren von Speicher für die Nachricht.\n");
            fclose(message_file);
            return 1;
        }
        // Nachricht lesen
        fread(message, 1, message_length, message_file);
        message[message_length] = '\0'; // Nullterminierungszeichen hinzufügen
        fclose(message_file);

    //Schlüssel einlesen
        FILE *file = fopen(privatekeyFileName, "r");
        if (!file) {
            perror("Fehler beim Öffnen der Datei");
            return 1;
        }
        EC_KEY *privateKey = PEM_read_ECPrivateKey(file, NULL, NULL, NULL);
        if (!privateKey) {
            fprintf(stderr, "Fehler beim Lesen des privaten Schlüssels: %s\n", ERR_error_string(ERR_get_error(), NULL));
            return 1;
        }
        fclose(file);

    //Hasherstellung
        unsigned char digest[SHA384_DIGEST_LENGTH];
        SHA384((const unsigned char *)message, strlen(message), digest);

    //Signierung des Hash
        ECDSA_SIG *signature;
        signature = ECDSA_do_sign(digest, SHA384_DIGEST_LENGTH, privateKey);
        if (signature == NULL)
            {
            printf("Fehler beim Signieren der Nachricht.\n");
            return 1;
            }

    //Speicherung der Signatur in einer TXT-Datei
        unsigned char *der_encoded_signature = NULL;
        int der_len = i2d_ECDSA_SIG(signature, &der_encoded_signature);
        if (der_len < 0) {
            fprintf(stderr, "Fehler beim Umwandeln der Signatur in DER-kodiertes Format.\n");
            return 1;
        }
        // Konvertieren der Signatur in eine hexadezimale Zeichenfolge
            char hex_signature[der_len * 2 + 1]; // Zwei Zeichen pro Byte
            for (int i = 0; i < der_len; i++) {
                sprintf(hex_signature + i * 2, "%02x", der_encoded_signature[i]);
            }
            hex_signature[der_len * 2] = '\0'; // Beenden der Zeichenfolge

            // Speichern der Zeichenfolge in einer Datei
            FILE *signfile = fopen(signatureFileName, "w");
            if (signfile == NULL) {
                fprintf(stderr, "Fehler beim Öffnen der Datei.\n");
                return 1;
            }
            fwrite(hex_signature, 1, strlen(hex_signature), signfile);
            fclose(signfile);

    // Bereinigungen
        ECDSA_SIG_free(signature);
        EC_KEY_free(privateKey);

        struct timespec end = timeMeasure();
        struct timespec diff = timespecDiff(begin, end);
        saveMeasurement(measurements, i, i, diff);


        printf("[SIG %i] Signatur erstellt.\n",i);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);
    return 0;
}
