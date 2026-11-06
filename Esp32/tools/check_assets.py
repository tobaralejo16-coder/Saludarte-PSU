"""Valida tabla, archivos de voz y presupuesto de SPIFFS antes de uploadfs.

Uso: python tools/check_assets.py
Solo biblioteca estandar de Python 3. No modifica archivos.
"""
from pathlib import Path
import re
import struct
import sys
import csv

ROOT = Path(__file__).resolve().parents[1]
SAFE_SPIFFS_FRACTION = 0.65
WAV_SAMPLE_RATE = 8000


def parse_partitions(path):
    partitions = {}
    with path.open(newline="", encoding="utf-8-sig") as source:
        for row in csv.reader(
                line for line in source if not line.lstrip().startswith("#")):
            if not row:
                continue
            if len(row) < 5:
                raise ValueError(f"Fila de particion incompleta: {row}")
            name, _, _, offset_text, size_text = (
                field.strip() for field in row[:5])
            offset = int(offset_text, 0)
            size = int(size_text, 0)
            if name in partitions or size <= 0:
                raise ValueError(f"Particion duplicada o vacia: {name}")
            partitions[name] = (offset, size)
    if "factory" not in partitions or "spiffs" not in partitions:
        raise ValueError("Se requieren particiones factory y spiffs")
    app_offset, app_size = partitions["factory"]
    spiffs_offset, spiffs_size = partitions["spiffs"]
    if app_offset + app_size > spiffs_offset:
        raise ValueError("Las particiones factory y spiffs se solapan")
    config = (ROOT / "include/Config.h").read_text(encoding="utf-8")
    expected_config = {
        "APP_PARTITION_OFFSET": app_offset,
        "APP_PARTITION_BYTES": app_size,
        "SPIFFS_PARTITION_OFFSET": spiffs_offset,
        "SPIFFS_PARTITION_BYTES": spiffs_size,
    }
    for name, expected in expected_config.items():
        configured = re.search(
            rf"{name}\s*=\s*(0x[0-9A-Fa-f]+|\d+)", config)
        if not configured or int(configured.group(1), 0) != expected:
            raise ValueError(f"{name} no coincide con partitions.csv")
    if spiffs_offset + spiffs_size > 0x400000:
        raise ValueError("Las particiones exceden la flash ESP32 de 4 MiB")
    return partitions


def validate_wav(path):
    data = path.read_bytes()
    if len(data) < 12 or data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("no es RIFF/WAVE")
    end = struct.unpack_from("<I", data, 4)[0] + 8
    if end > len(data) or end < 12:
        raise ValueError("RIFF truncado")
    pos, fmt, pcm = 12, None, None
    while pos < end:
        if end - pos < 8:
            raise ValueError("cabecera de chunk truncada")
        kind, count = struct.unpack_from("<4sI", data, pos)
        pos += 8
        next_pos = pos + count + (count & 1)
        if next_pos > end:
            raise ValueError("chunk fuera del RIFF")
        if kind == b"fmt ":
            if fmt is not None or count < 16:
                raise ValueError("fmt duplicado/incompleto")
            fmt = struct.unpack_from("<HHIIHH", data, pos)
            encoding, channels, rate, byte_rate, block_align, bits = fmt
            sample_bytes = bits // 8
            if (encoding != 1 or channels != 1 or rate != WAV_SAMPLE_RATE or
                    bits not in (8, 16) or block_align != sample_bytes or
                    byte_rate != rate * block_align):
                raise ValueError("se requiere PCM mono, 8/16 bits, 8000 Hz")
        elif kind == b"data":
            if pcm is not None or not count:
                raise ValueError("data duplicado o vacio")
            pcm = count
        pos = next_pos
    if fmt is None or pcm is None:
        raise ValueError("faltan fmt o data")
    sample_bytes = fmt[5] // 8
    if pcm % sample_bytes:
        raise ValueError("data desalineado")
    return pcm / (WAV_SAMPLE_RATE * sample_bytes)


def parse_activities(text):
    """Valida todas las filas; evita ignorar una actividad mal escrita."""
    entries, ids, names, codes, paths = [], set(), set(), set(), set()
    pattern = (r'ACTIVIDAD\(\s*([A-Z][A-Z0-9_]*)\s*,\s*"([0-9]{2})"\s*,'
               r'\s*([1-9][0-9]*)\s*,\s*"([^"\n]+)"\s*\)')
    for line in text.splitlines():
        line = line.split("//", 1)[0].strip()
        if not line:
            continue
        match = re.fullmatch(pattern, line)
        if not match:
            raise ValueError(f"Fila de actividades invalida: {line}")
        name, code, id_text, path = match.groups()
        identifier = int(id_text)
        if (identifier != int(code) or not 1 <= identifier <= 99 or
                identifier in ids or name in names or code in codes or path in paths):
            raise ValueError(f"Actividad duplicada o ID/codigo incorrecto: {name}")
        if not re.fullmatch(r"/audio/[A-Za-z0-9_-]+\.wav", path) or len(path) > 31:
            raise ValueError(f"Ruta SPIFFS invalida: {path}")
        ids.add(identifier)
        names.add(name)
        codes.add(code)
        paths.add(path)
        entries.append((name, code, identifier, path))
    if not entries:
        raise ValueError("Tabla vacia")
    return entries


def main():
    try:
        partitions = parse_partitions(ROOT / "partitions.csv")
        entries = parse_activities((ROOT / "include/Activities.def").read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        print("ERROR:", error, file=sys.stderr)
        return 1
    errors = []
    for name, code, identifier, relative in entries:
        path = ROOT / "data" / relative.lstrip("/")
        try:
            seconds = validate_wav(path)
            print(f"OK {code} {name:15} {relative}: {seconds:.2f} s")
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    total = sum(p.stat().st_size for p in (ROOT / "data").rglob("*") if p.is_file())
    # Margen conservador para metadatos/bloques; buildfs confirma el limite real.
    budget = int(partitions["spiffs"][1] * SAFE_SPIFFS_FRACTION)
    print(f"Particion SPIFFS: {partitions['spiffs'][1]} bytes")
    print(f"Archivos: {total} bytes; presupuesto recomendado: {budget} bytes")
    if total > budget:
        errors.append("Se excede el presupuesto seguro recomendado para SPIFFS")
    for error in errors:
        print("ERROR:", error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
