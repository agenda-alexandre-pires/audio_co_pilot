#!/bin/bash

echo "=== Audio Co-Pilot - Monitor de Logs ==="
echo ""

# Mata processos anteriores
killall "Audio Co-Pilot" 2>/dev/null
sleep 1

# Remove logs antigos
rm -f ~/Desktop/AudioCoPilot_*.txt

# Inicia o app
echo "Iniciando Audio Co-Pilot..."
open "/Users/emersonporfa/Desktop/Audio_Co_Pilot/build/AudioCoPilot_artefacts/Release/Audio Co-Pilot.app"

# Aguarda o log ser criado
echo "Aguardando logs..."
sleep 8

# Mostra os logs
LOG_FILE=$(ls -t ~/Desktop/AudioCoPilot_*.txt 2>/dev/null | head -1)

if [ -f "$LOG_FILE" ]; then
    echo ""
    echo "=== LOGS DO AUDIO CO-PILOT ==="
    echo "Arquivo: $LOG_FILE"
    echo ""
    tail -100 "$LOG_FILE"
    echo ""
    echo "=== MONITORANDO (pressione Ctrl+C para parar) ==="
    tail -f "$LOG_FILE"
else
    echo "Arquivo de log não encontrado. Aguarde mais alguns segundos..."
    sleep 5
    LOG_FILE=$(ls -t ~/Desktop/AudioCoPilot_*.txt 2>/dev/null | head -1)
    if [ -f "$LOG_FILE" ]; then
        tail -100 "$LOG_FILE"
        tail -f "$LOG_FILE"
    fi
fi
