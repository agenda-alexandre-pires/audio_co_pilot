#!/bin/bash

cd /Users/emersonporfa/Desktop/Audio_Co_Pilot

# Encerrar instâncias anteriores
killall "Audio Co-Pilot" 2>/dev/null
sleep 1

# Verificar se app existe
if [ ! -f "build/AudioCoPilot_artefacts/Release/Audio Co-Pilot.app/Contents/MacOS/Audio Co-Pilot" ]; then
    echo "Compilando..."
    cd build
    cmake --build . --config Release
    cd ..
fi

# Executar o app
echo "Abrindo Audio Co-Pilot..."
open "build/AudioCoPilot_artefacts/Release/Audio Co-Pilot.app"

# Verificar se está rodando
sleep 3
if ps aux | grep -i "Audio Co-Pilot" | grep -v grep > /dev/null; then
    echo "✓ Programa está rodando!"
else
    echo "✗ Programa não iniciou"
fi
