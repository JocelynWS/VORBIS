import pygame

# Khởi tạo pygame và mixer
pygame.init()
pygame.mixer.init()

# Tên file MIDI (thay bằng tên file bạn muốn phát)
midi_file = "random_mido.mid"

# Tải file MIDI
pygame.mixer.music.load(midi_file)

# Phát file MIDI
print("🎵 Đang phát file MIDI...")
pygame.mixer.music.play()

# Chờ cho đến khi phát xong
while pygame.mixer.music.get_busy():
    pygame.time.wait(100)

print("✅ Đã phát xong.")
pygame.mixer.quit()
