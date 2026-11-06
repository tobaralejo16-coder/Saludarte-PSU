"""Pruebas del verificador de archivos del ordenador; no sustituyen a la placa."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import wave

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from check_assets import validate_wav, parse_activities, parse_partitions
from prepare_audio import convert_wav


def chunk(kind, payload):
    return kind + struct.pack("<I", len(payload)) + payload + b"\0" * (len(payload) % 2)


FMT16 = chunk(b"fmt ", struct.pack("<HHIIHH", 1, 1, 8000, 16000, 2, 16))
FMT8 = chunk(b"fmt ", struct.pack("<HHIIHH", 1, 1, 8000, 8000, 1, 8))
PCM16 = chunk(b"data", b"\x01\x00" * 160)
PCM8 = chunk(b"data", bytes(range(160)))


def riff(*chunks):
    body = b"WAVE" + b"".join(chunks)
    return b"RIFF" + struct.pack("<I", len(body)) + body


class AssetTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.path = Path(self.temp.name) / "audio.wav"

    def tearDown(self):
        self.temp.cleanup()

    def validate(self, data):
        self.path.write_bytes(data)
        return validate_wav(self.path)

    def test_valid_pcm(self):
        self.assertAlmostEqual(self.validate(riff(FMT16, PCM16)), 0.02)

    def test_metadata_and_odd_padding(self):
        self.assertAlmostEqual(self.validate(riff(chunk(b"JUNK", b"abc"), FMT8, PCM8)), 0.02)

    def test_data_before_fmt(self):
        self.assertAlmostEqual(self.validate(riff(PCM8, FMT8)), 0.02)

    def test_pcm8_odd_data_length(self):
        odd_pcm = chunk(b"data", b"\x80" * 159)
        self.assertAlmostEqual(self.validate(riff(FMT8, odd_pcm)), 159 / 8000)

    def test_truncated_at_every_byte(self):
        valid = riff(FMT16, PCM16)
        for end in range(len(valid)):
            with self.subTest(end=end), self.assertRaises(ValueError):
                self.validate(valid[:end])

    def test_unsupported_format_fields(self):
        fields = [1, 1, 8000, 16000, 2, 16]
        for index, bad in enumerate([3, 2, 16000, 123, 4, 24]):
            changed = fields.copy()
            changed[index] = bad
            with self.subTest(index=index), self.assertRaises(ValueError):
                self.validate(riff(chunk(b"fmt ", struct.pack("<HHIIHH", *changed)), PCM16))
    def test_bad_containers(self):
        bad_cases = [b"ID3" + bytes(100), riff(FMT16), riff(PCM16), riff(),
                     riff(FMT16, FMT16, PCM16), riff(FMT16, PCM16, PCM16),
                     riff(FMT16, chunk(b"data", b"")),
                     riff(FMT16, chunk(b"data", b"x")),
                     riff(chunk(b"fmt ", b"short"), PCM16),
                     riff(FMT16, b"data\xff\xff\xff\xff"),
                     riff(FMT16, PCM16, b"tail")]
        for index, data in enumerate(bad_cases):
            with self.subTest(index=index), self.assertRaises(ValueError):
                self.validate(data)

    def test_pcm16_to_pcm8_conversion(self):
        source = self.path.parent / "source.wav"
        with wave.open(str(source), "wb") as wav:
            wav.setnchannels(1)
            wav.setsampwidth(2)
            wav.setframerate(8000)
            wav.writeframes(struct.pack("<hhhhh", -32768, -256, 0, 255, 32767))
        convert_wav(source, self.path)
        with wave.open(str(self.path), "rb") as wav:
            self.assertEqual((wav.getnchannels(), wav.getsampwidth(), wav.getframerate()),
                             (1, 1, 8000))
            self.assertEqual(wav.getnframes(), 6)
            self.assertEqual(wav.readframes(6), bytes([0, 127, 128, 129, 255, 128]))
        self.assertEqual(source.stat().st_size, 54)

    def test_tone_and_no_overwrite(self):
        script = Path(__file__).resolve().parents[1] / "tools/make_test_tone.py"
        first = subprocess.run([sys.executable, str(script), str(self.path)], capture_output=True)
        self.assertEqual(first.returncode, 0, first.stderr.decode(errors="replace"))
        self.assertEqual(validate_wav(self.path), 1.0)
        before = self.path.read_bytes()
        second = subprocess.run([sys.executable, str(script), str(self.path)], capture_output=True)
        self.assertNotEqual(second.returncode, 0)
        self.assertEqual(self.path.read_bytes(), before)


class ActivityTests(unittest.TestCase):
    def test_android_mapping(self):
        table = Path(__file__).resolve().parents[1] / "include/Activities.def"
        entries = parse_activities(table.read_text(encoding="utf-8"))
        expected = ["DESPERTAR", "DESAYUNAR", "BANARSE", "CAMINATA", "MERENDAR",
                    "ALMORZAR", "CENAR", "TALLER", "DORMIR", "MEDICAMENTO",
                    "CEPILLARSE", "DESCANSO", "EMERGENCIA"]
        identifiers = list(range(1, 13)) + [99]
        self.assertEqual(entries, [(name, f"{id_:02}", id_, f"/audio/{id_:04}.wav")
                                   for name, id_ in zip(expected, identifiers)])

    def test_rejects_invalid_catalog(self):
        valid = 'ACTIVIDAD(DESPERTAR, "01", 1, "/audio/0001.wav")'
        for text in ["", valid + "\n" + valid, valid.replace('"01"', '"1"'),
                     valid.replace(', 1,', ', 2,'), valid.replace(', 1,', ', 01,'),
                     valid.replace('/audio/0001.wav', '../0001.wav'), "invalid"]:
            with self.subTest(text=text), self.assertRaises(ValueError):
                parse_activities(text)

    def test_new_activity(self):
        self.assertEqual(parse_activities('ACTIVIDAD(LEER, "13", 13, "/audio/0013.wav")'),
                         [("LEER", "13", 13, "/audio/0013.wav")])


class PartitionTests(unittest.TestCase):
    def test_spiffs_partition_matches_config_and_fits_flash(self):
        root = Path(__file__).resolve().parents[1]
        partitions = parse_partitions(root / "partitions.csv")
        self.assertEqual(partitions["factory"], (0x10000, 0x0F0000))
        self.assertEqual(partitions["spiffs"], (0x100000, 0x300000))
        self.assertLessEqual(partitions["spiffs"][0] + partitions["spiffs"][1],
                             0x400000)

    def test_rejects_overlapping_partitions(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "partitions.csv"
            path.write_text(
                "factory, app, factory, 0x10000, 0x100000,\n"
                "spiffs, data, spiffs, 0x100000, 0x300000,\n",
                encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "solapan"):
                parse_partitions(path)


if __name__ == "__main__":
    unittest.main()
