#!/bin/bash
controlfolder=/opt/system/Tools/PortMaster
source "$controlfolder/control.txt"
gamedir="$(cd -- "$(dirname -- "$(readlink -f -- "$0")")" && pwd)/th01port"
cd "$gamedir" || exit 1
exec >log.txt 2>&1
get_controls
export SDL_VIDEODRIVER=kmsdrm
export SDL_AUDIODRIVER=alsa
export TH01_AUDIO_MONO=1
export TH01_START_KEY_ONLY=1
export SDL_GAMECONTROLLERCONFIG_FILE="$controlfolder/gamecontrollerdb.txt"
unset TH01_WAV_DIR
if [[ "${TH01_AUDIO_BACKEND:-opna}" == midi ]]; then
  export TH01_MIDI_SOUNDFONT="${TH01_MIDI_SOUNDFONT:-/usr/share/sounds/sf2/TimGM6mb.sf2}"
else
  unset TH01_MIDI_SOUNDFONT
fi
mapper_pid=""
export TH01_HEADPHONE_JACK_INVERT=1
source "$gamedir/audio-route.sh"
cleanup() {
  th01_audio_stop
  if [[ -n "$mapper_pid" ]]; then
    $ESUDO kill -TERM -- "-$mapper_pid" 2>/dev/null || true
    wait "$mapper_pid" 2>/dev/null || true
  fi
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
th01_audio_start
setsid $GPTOKEYB "th01-native" -c "$gamedir/th01.gptk" &
mapper_pid=$!
./th01-native assets "$@"
exit $?
