#!/bin/bash

echo "========================================"
echo "Audio Co-Pilot - Executando com logs no terminal"
echo "========================================"
echo ""

# Mata processos anteriores
killall "Audio Co-Pilot" 2>/dev/null
sleep 1

# Remove logs antigos
rm -f ~/Desktop/AudioCoPilot_*.txt

# Executa o binário diretamente (mostra logs no terminal)
echo "Iniciando Audio Co-Pilot..."
echo "Os logs aparecerão abaixo:"
echo "========================================"
echo ""

"/Users/emersonporfa/Desktop/Audio_Co_Pilot/build/AudioCoPilot_artefacts/Release/Audio Co-Pilot.app/Contents/MacOS/Audio Co-Pilot"
