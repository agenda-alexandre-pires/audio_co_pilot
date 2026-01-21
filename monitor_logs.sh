#!/bin/bash

echo "========================================"
echo "Audio Co-Pilot - Monitor de Logs"
echo "========================================"
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
echo "Aguardando logs (10 segundos)..."
sleep 10

# Encontra o arquivo de log mais recente
LOG_FILE=$(ls -t ~/Desktop/AudioCoPilot_*.txt 2>/dev/null | head -1)

if [ -f "$LOG_FILE" ]; then
    echo ""
    echo "=== LOGS DO AUDIO CO-PILOT ==="
    echo "Arquivo: $LOG_FILE"
    echo ""
    cat "$LOG_FILE"
    echo ""
    echo "=== MONITORANDO EM TEMPO REAL (Ctrl+C para parar) ==="
    echo ""
    tail -f "$LOG_FILE"
else
    echo "Arquivo de log não encontrado após 10 segundos."
    echo "Tentando novamente..."
    sleep 5
    LOG_FILE=$(ls -t ~/Desktop/AudioCoPilot_*.txt 2>/dev/null | head -1)
    if [ -f "$LOG_FILE" ]; then
        cat "$LOG_FILE"
        tail -f "$LOG_FILE"
    else
        echo "Erro: Não foi possível encontrar o arquivo de log."
    fi
fi
