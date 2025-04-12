from mido import Message, MidiFile, MidiTrack, MetaMessage, bpm2tempo
import random

def create_output_midi(filename="output_midi.mid", bpm=55, duration_seconds=180):
    mid = MidiFile()
    ticks_per_beat = mid.ticks_per_beat
    tempo = bpm2tempo(bpm)

    beats_per_second = bpm / 60
    total_beats = int(duration_seconds * beats_per_second)

    # Tempo track
    tempo_track = MidiTrack()
    tempo_track.append(MetaMessage("set_tempo", tempo=tempo, time=0))  
    mid.tracks.append(tempo_track)

    # Instruments
    instruments = {
        "piano": (0, [60, 64, 67]),
        "guitar": (24, [55, 59, 62]),
        "pad": (89, [48, 52]),
        "bass": (33, [40, 43]),
        "violin": (40, [72, 74, 76, 77, 79])
    }

    def make_track(channel, program, notes, density=0.5):
        track = MidiTrack()
        track.append(Message("program_change", program=program, channel=channel, time=0))
        time = 0
        for _ in range(total_beats):
            if random.random() < density:
                note = random.choice(notes)
                track.append(Message("note_on", note=note, velocity=70, time=time, channel=channel))
                track.append(Message("note_off", note=note, velocity=70, time=ticks_per_beat, channel=channel))
                time = 0
            else:
                time += ticks_per_beat
        mid.tracks.append(track)

    for idx, (key, (program, notes)) in enumerate(instruments.items()):
        density = 0.6 if key == 'violin' else 0.4
        make_track(idx, program, notes, density)

    mid.save(filename)
    print(f"✅ Đã tạo file MIDI: {filename}")

# Gọi hàm
duration = float(input("⏱ Nhập thời lượng bản nhạc (giây): "))
create_output_midi("output_midi.mid", bpm=55, duration_seconds=duration)
