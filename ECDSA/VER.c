#include <stdio.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include "../timeMeasure.h"

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

    // Hash der Nachricht berechnen
        unsigned char digest[SHA384_DIGEST_LENGTH];
        SHA384((const unsigned char *)message, strlen(message), digest);

    //Signatur als TXT laden:
        // Datei einlesen
        FILE *file = fopen(signatureFileName, "r");
        if (file == NULL) {
            fprintf(stderr, "Fehler beim Öffnen der Datei.\n");
            return 1;
        }

        // Größe der Datei ermitteln
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        // Speicher für die hexadezimale Zeichenfolge reservieren
        char *hex_signature = malloc(file_size + 1);
        if (hex_signature == NULL) {
            fprintf(stderr, "Fehler beim Speichern der Signatur.\n");
            return 1;
        }

        // Dateiinhalt in die Zeichenfolge lesen
        fread(hex_signature, 1, file_size, file);
        hex_signature[file_size] = '\0'; // Beenden der Zeichenfolge
        fclose(file);

    //Signatur laden
        int signature_len = strlen(hex_signature) / 2;
        // Byte-Array für die Signatur vorbereiten
        unsigned char *signature_bytes = malloc(signature_len);
        if (!signature_bytes) {
            fprintf(stderr, "Fehler beim Speichern der Signatur.\n");
            free(hex_signature);
            return 1;
        }
    // Hexadezimale Zeichenfolge in Byte-Array umwandeln
        for (int i = 0; i < signature_len; i++) {
            unsigned int temp; // Temporärer Wert für das Lesen der Hexadezimalzahl
            sscanf(hex_signature + i * 2, "%02x", &temp);
            signature_bytes[i] = (unsigned char)temp; // Konvertieren in unsigned char
        }

    // Die Signatur in eine ECDSA_SIG-Struktur umwandeln
        const unsigned char *p = signature_bytes;
        ECDSA_SIG *signature = d2i_ECDSA_SIG(NULL, &p, signature_len);
        if (!signature) {
            fprintf(stderr, "Fehler beim Umwandeln der Signatur.\n");
            free(signature_bytes);
            return 1;
        }

    // Öffentlichen Schlüssel aus der .pem-Datei laden
    FILE *publicKeyFile = fopen(publicKeyFileName, "r");
    if (!publicKeyFile) {
        perror("Fehler beim Öffnen der .pem-Datei");
        return 1;
    }
    EC_KEY *publicKey = PEM_read_EC_PUBKEY(publicKeyFile, NULL, NULL, NULL);
    fclose(publicKeyFile);
    if (!publicKey) {
        fprintf(stderr, "Fehler beim Lesen des öffentlichen Schlüssels: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return 1;
    }

    // Signatur validieren
    int result = ECDSA_do_verify(digest, SHA384_DIGEST_LENGTH, signature, publicKey);
    if (result == 1) {
        printf("[VER %i] Die ECDSA-Signatur ist gültig.\n", i);
    } else {
        printf("[VER %i] Die ECDSA-Signatur ist ungültig.\n", i);
    }

    // Ressourcen freigeben
    ECDSA_SIG_free(signature);
    EC_KEY_free(publicKey);

    struct timespec end = timeMeasure();
    struct timespec diff = timespecDiff(begin, end);
    saveMeasurement(measurements, i, i, diff);
    }

    writeMeasurementsToCSV(runTimeFilename, measurements, iterations);
    freeMeasurements(measurements);

    return 0;
    
}
