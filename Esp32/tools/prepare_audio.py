"""Reduce WAV PCM16 de origen a PCM8 para que todas las voces quepan en SPIFFS.

Uso: python tools/prepare_audio.py
Los originales se guardan en assets/audio_source/; data/audio/ contiene los WAV
compactos que PlatformIO carga al ESP32.
"""
from pathlib import Path
import struct
import sys
import tempfile
import wave

from check_assets import parse_activities, validate_wav

ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "assets" / "audio_source"
OUTPUT_DIR = ROOT / "data" / "audio"
SAMPLE_RATE = 8000
BLOCK_FRAMES = 4096


def to_unsigned_pcm8(pcm16):
    converted = bytearray()
    if len(pcm16) % 2:
        raise ValueError("bloque PCM16 desalineado")
    for (sample,) in struct.iter_unpack("<h", pcm16):
        if sample >= 0:
            quantized = (sample + 128) // 256
        else:
            quantized = -((-sample + 128) // 256)
        quantized = max(-128, min(127, quantized))
        converted.append(quantized + 128)
    return converted


def convert_wav(source, target):
    validate_wav(source)
    target.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(source), "rb") as reader:
        if (reader.getnchannels() != 1 or reader.getsampwidth() != 2 or
                reader.getframerate() != SAMPLE_RATE or
                reader.getcomptype() != "NONE"):
            raise ValueError(
                f"{source.name}: el original debe ser PCM16 mono a 8000 Hz"
            )
        with tempfile.NamedTemporaryFile(
                dir=target.parent, suffix=".wav.tmp", delete=False) as temp:
            temp_path = Path(temp.name)
        try:
            with wave.open(str(temp_path), "wb") as writer:
                writer.setnchannels(1)
                writer.setsampwidth(1)
                writer.setframerate(SAMPLE_RATE)
                while True:
                    frames = reader.readframes(BLOCK_FRAMES)
                    if not frames:
                        break
                    writer.writeframesraw(to_unsigned_pcm8(frames))
                if reader.getnframes() % 2:
                    writer.writeframesraw(b"\x80")
            temp_path.replace(target)
        except Exception:
            temp_path.unlink(missing_ok=True)
            raise


def main():
    try:
        entries = parse_activities(
            (ROOT / "include" / "Activities.def").read_text(encoding="utf-8")
        )
        sources = []
        for _, _, _, relative in entries:
            source = SOURCE_DIR / Path(relative).name
            validate_wav(source)
            sources.append((source, OUTPUT_DIR / Path(relative).name))
        for source, target in sources:
            convert_wav(source, target)
            print(f"OK {source.name} -> {target.relative_to(ROOT)}")
    except (OSError, ValueError, wave.Error) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1
    print(f"Voces compactadas: {len(sources)}. Los originales no se modificaron.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
