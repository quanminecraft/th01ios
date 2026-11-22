"""Prepare the requested recorded gamerip as verified PCM WAV plus loop metadata.

Run from the repository root. Host-only dependencies: soundfile and numpy in
build/audio-tools. Direct URLs were read from the album's visible download links;
no login, scraper challenge bypass, or original game files are required here.
"""
from pathlib import Path
import runpy
for script in ('download_recordings.py', 'find_recorded_loops.py'):
    runpy.run_path(str(Path(__file__).with_name(script)), run_name='__main__')
