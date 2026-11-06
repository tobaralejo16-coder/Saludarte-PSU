"""Genera un WAV PCM8 de prueba, NO una voz. No sobrescribe archivos existentes.
Uso: python tools/make_test_tone.py data/audio/0001.wav
"""
import math
from pathlib import Path
import struct
import sys
import wave

if len(sys.argv) != 2:
    raise SystemExit("Uso: python tools/make_test_tone.py ruta.wav")
path = Path(sys.argv[1])
path.parent.mkdir(parents=True, exist_ok=True)
samples = bytearray()
for i in range(8000):
    # Tono 440 Hz, un segundo, amplitud baja y fundidos de 20 ms.
    envelope = min(1.0, i / 160, (7999 - i) / 160)
    value = int(13 * envelope * math.sin(2 * math.pi * 440 * i / 8000))
    samples.append(value + 128)
with path.open("xb") as target:
    with wave.open(target, "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(1)
        wav.setframerate(8000)
        wav.writeframes(samples)
print(f"Tono de prueba creado: {path}")
