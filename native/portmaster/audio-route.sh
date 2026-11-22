#!/bin/bash
# Sourced only by the TH01 launcher. No persistent/system mixer changes.
th01_audio_path() {
  LC_ALL=C amixer -c 0 cget 'name=Playback Path' 2>/dev/null |
    sed -n 's/^  : values=\([0-3]\)$/\1/p'
}
th01_audio_watch() {
  local owner="$1" original current jack wanted last=""
  original="$(th01_audio_path)"
  case "$original" in 0|1|2|3) ;; *) return ;; esac
  th01_audio_restore() {
    current="$(th01_audio_path)"
    # Restore only a route this helper still owns, leaving external changes alone.
    if [[ -n "$last" && "$current" == "$last" ]]; then
      amixer -q -c 0 cset 'name=Playback Path' "$original" 2>/dev/null || true
    fi
  }
  trap th01_audio_restore EXIT
  trap 'exit 0' HUP INT TERM
  while kill -0 "$owner" 2>/dev/null; do
    jack="$(LC_ALL=C amixer -c 0 cget 'iface=CARD,name=Headphone Jack' 2>/dev/null)"
    case "$jack" in
      *': values=on'*) wanted=2 ;; # HP only: external speaker disabled
      *': values=off'*) wanted=1 ;; # SPK only
      *) sleep 0.5; continue ;; # Unknown status must not enable the speaker.
    esac
    # MyMini RK817 jack sense is inverted: unplug=on, inserted=off.
    if [[ "${TH01_HEADPHONE_JACK_INVERT:-0}" == 1 ]]; then
      if [[ "$wanted" == 2 ]]; then wanted=1; else wanted=2; fi
    fi
    current="$(th01_audio_path)"
    if [[ "$current" != "$wanted" ]]; then
      if amixer -q -c 0 cset 'name=Playback Path' "$wanted" 2>/dev/null; then
        last="$wanted"
      fi
    fi
    sleep 0.5
  done
}
th01_audio_start() {
  th01_audio_pid=""
  command -v amixer >/dev/null 2>&1 || return 0
  th01_audio_watch "$$" &
  th01_audio_pid=$!
}
th01_audio_stop() {
  if [[ -n "${th01_audio_pid:-}" ]]; then
    kill -TERM "$th01_audio_pid" 2>/dev/null || true
    wait "$th01_audio_pid" 2>/dev/null || true
    th01_audio_pid=""
  fi
}
