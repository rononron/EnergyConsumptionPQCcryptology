#!/bin/bash

# Überprüfe, ob ein Eingabeparameter vorhanden ist
if [ "$#" -ne 2 ]; then
    echo "<Prefix> <STEP>"
    exit 1
fi

# Eingabeparameter als Dateinamen-Präfix übernehmen
prefix="$1"
step="$2"

# Dateiname für die Ausgabedatei
output_file="${prefix}_${step}_ergebnisse.txt"

# Verwendung von find, um die spezifische Datei zu finden
input_file="${prefix}_${step}_messung"

# Überprüfen, ob eine Datei gefunden wurde
if [ ! -f "$input_file" ]; then
    echo "Messergebnis-Datei nicht gefunden."
    exit 1
fi

    # Dateinamen für die Zwischendatei
    temp_file_Tasks="${prefix}_${step}_tempTasks.txt"
    temp_file_cpu="${prefix}_${step}_tempCPU.txt"
    temp_file_gpu="${prefix}_${step}_tempGPU.txt"
    temp_file_ane="${prefix}_${step}_tempANE.txt"


    # Befehl zur Extraktion der Daten
    grep_result=$(grep "$step" "$input_file" | awk '{print $3, $14, $15, $20, $29, $30, $32, $33, $34, $35, $36}')

# Überprüfen, ob die Ausgabe leer ist
    if [ -z "$grep_result" ]; then
        echo "Fehler: Schritt $step nicht in der Eingabedatei gefunden." >> "$output_file"
        exit 1
    fi
    echo "$grep_result" > "$temp_file_Tasks"

        # Spalten, für die der Durchschnitt berechnet werden soll
        columns="1 2 3 4 5 6 8 9 10 11"

        # Summen und Durchschnitte der Prozess-Informationen berechnen
        cpu_ms_per_s=$(awk '{ sum += $1; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        bytes_read=$(awk '{ sum += $2 } END { printf "%.2f", sum }' "$temp_file_Tasks") #Summe
        bytes_written=$(awk '{ sum += $3 } END { printf "%.2f", sum }' "$temp_file_Tasks") #Summe
        gpu_ms_per_s=$(awk '{ sum += $4; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        cpuPrimCore_ms_per_s=$(awk '{ sum += $5; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        cpuPrimCore_percent=$(awk '{ sum += $6; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        energy_impact=$(awk '{ sum += $7 } END { printf "%.2f", sum }' "$temp_file_Tasks") #Summe
        instr_per_s=$(awk '{ sum += $8; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        cycles_per_s=$(awk '{ sum += $9; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        p_instr_per_s=$(awk '{ sum += $10; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        p_cycles_per_s=$(awk '{ sum += $11; count++ } END { printf "%.2f", sum/count }' "$temp_file_Tasks")
        #rm "$temp_file_Tasks"

        awk -F ': ' '/CPU Power:/ {print $2}' "$input_file" | cut -d ' ' -f1 > "$temp_file_cpu"
        cpuPower=$(awk '{sum+=$1} END {print sum}' "$temp_file_cpu")
        #rm "$temp_file_cpu"

        awk -F ': ' '/GPU Power:/ {print $2}' "$input_file" | cut -d ' ' -f1 > "$temp_file_gpu"
        gpuPower=$(awk '{sum+=$1} END {print sum}' "$temp_file_gpu")
        #rm "$temp_file_gpu"

        awk -F ': ' '/ANE Power:/ {print $2}' "$input_file" | cut -d ' ' -f1 > "$temp_file_ane"
        anePower=$(awk '{sum+=$1} END {print sum}' "$temp_file_ane")
        #rm "$temp_file_ane"

    # Ausgabe der aktuellen Werte
    echo "Prozess" >> "$output_file"
        echo "Durchschnitt CPU (ms/s): $cpu_ms_per_s" >> "$output_file"
        echo "Summe Bytes Writen: $bytes_read" >> "$output_file"
        echo "Summe Bytes Read: $bytes_written" >> "$output_file"
        echo "Durchschnitt GPU Nutzung ms/s: $gpu_ms_per_s" >> "$output_file"
        echo "Durchschnitt CPU primärer Kern Verwendung ms/s: $cpuPrimCore_ms_per_s" >> "$output_file"
        echo "Durchschnitt CPU primärer Kern Verwendung in %: $cpuPrimCore_percent" >> "$output_file"
        echo "Summe Energy Impact (mW): $energy_impact" >> "$output_file"
        echo "Durchschnitt Instr/s: $instr_per_s" >> "$output_file"
        echo "Durchschnitt Cycles/s: $cycles_per_s" >> "$output_file"
        echo "Durchschnitt P-Instr/s: $p_instr_per_s" >> "$output_file"
        echo "Durchschnitt P-Cycles/s: $p_cycles_per_s" >> "$output_file"
    echo "System" >> "$output_file"
        echo "Summe CPU-Power: $cpuPower" >> "$output_file"
        echo "Summe GPU-Power: $gpuPower" >> "$output_file"
        echo "Summe ANE-Power: $anePower" >> "$output_file"