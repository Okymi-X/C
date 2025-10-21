#!/bin/bash

echo "========================================"
echo "  COMPILATION MESSAGERIE - LINUX"
echo "========================================"

# Compilation pour Linux
gcc -o messagerie_linux main_multiplatform.c fonctions.c -lm -D__linux__

if [ $? -eq 0 ]; then
    echo ""
    echo "[OK] Compilation Linux reussie!"
    echo "Executable: messagerie_linux"
    echo ""
    echo "Pour lancer:"
    echo "  ./messagerie_linux"
    echo ""
else
    echo ""
    echo "[ERREUR] Echec de la compilation"
    exit 1
fi
