import ffmpeg

# Giảm âm lượng nhạc nền
bgm = ffmpeg.input('output_midi.wav').audio.filter('volume', 1)
voice = ffmpeg.input('input.wav').audio.filter('volume', 0.9)

# Mix hai file audio lại với nhau
(
    ffmpeg
    .filter([voice, bgm], 'amix', inputs=2, duration='first', dropout_transition=3)
    .output('mixed.wav', ac=2)
    .overwrite_output()
    .run()
)
