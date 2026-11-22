"""Copy the user's OPNA percussion samples into the native asset directory."""
import argparse
import hashlib
from pathlib import Path
import shutil
import wave

parser = argparse.ArgumentParser()
parser.add_argument("source", type=Path, help="Directory containing 2608_*.WAV")
parser.add_argument("destination", type=Path)
args = parser.parse_args()
names = [f"2608_{name}.WAV" for name in ("BD", "SD", "TOP", "HH", "TOM", "RIM")]
for name in names:
    source = args.source / name
    if source.resolve() == (args.destination / name).resolve():
        raise SystemExit("Source and destination must differ")
    with wave.open(str(source)) as audio:
        if (audio.getnchannels(), audio.getsampwidth(), audio.getframerate(), audio.getcomptype()) != (1, 2, 48000, "NONE"):
            raise SystemExit(f"Unsupported rhythm format: {source}")
args.destination.mkdir(parents=True, exist_ok=True)
for name in names:
    source, destination = args.source / name, args.destination / name
    shutil.copyfile(source, destination)
    assert hashlib.sha256(source.read_bytes()).digest() == hashlib.sha256(destination.read_bytes()).digest()
print("Copied six original user-supplied OPNA percussion WAV assets")
