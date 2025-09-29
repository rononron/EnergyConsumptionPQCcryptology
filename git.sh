#!/bin/bash

# Überprüfen, ob genau ein Argument übergeben wurde
if [ $# -ne 1 ]; then
    echo "Commit-Infos fehlen"
    exit 1
fi

# Alle Dateien hinzufügen
git add -A

# Commit mit der übergebenen Nachricht
git commit -a -m "$1"

# Push zu origin/main
git push -u origin main
