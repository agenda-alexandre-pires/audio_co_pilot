#!/bin/bash

# Script para executar Audio Co-Pilot com logs no terminal

cd /Users/emersonporfa/Desktop/Audio_Co_Pilot

# Mata processos anteriores
killall "Audio Co-Pilot" 2>/dev/null
sleep 1

# Remove logs antigos
rm -f ~/Desktop/AudioCoPilot_*.txt

echo "========================================"
echo "Audio Co-Pilot - Logs no Terminal"
echo "========================================"
echo ""
echo "Executando o programa..."
echo "Os logs aparecerão abaixo em tempo real:"
echo "========================================"
echo ""

# Executa o binário diretamente - os logs aparecerão no terminal
"/Users/emersonporfa/Desktop/Audio_Co_Pilot/build/AudioCoPilot_artefacts/Release/Audio Co-Pilot.app/Contents/MacOS/Audio Co-Pilot"
