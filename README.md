---

# RetaOS

RetaOS, modern ve modüler bir işletim sistemi projesidir. Temel amacı, açık kaynaklı ve öğrenmesi kolay bir işletim sistemi mimarisi sunmaktır.

## Özellikler

- **Minimal Kernel:** Basit ve anlaşılır çekirdek mimarisi
- **Multiboot Uyumlu:** GRUB ile uyumlu boot sistemi
- **VGA Terminal:** Basit terminal çıktısı
- **QEMU Desteği:** Sanal makinede çalıştırma
- **Kolay Kurulum:** Hızlı ve pratik kurulum desteği

## Gereksinimler

Aşağıdaki paketlerin yüklü olması gerekiyor:

```bash
sudo apt install -y gcc-multilib build-essential nasm xorriso mtools qemu-system-x86 grub-pc-bin
```

## Kurulum ve Çalıştırma

### 1. Projeyi Derleme

```bash
make all
```

Bu komut:
- Kernel'i derler
- ISO dosyası oluşturur
- `build/retaos.iso` dosyasını üretir

### 2. İşletim Sistemini Çalıştırma

#### Grafik Modunda:
```bash
make run-gfx
# veya
./run.sh gfx
```

#### Seri Konsol Modunda:
```bash
make run
# veya
./run.sh serial
```

#### Debug Modunda:
```bash
./run.sh debug
```

### 3. Temizlik

```bash
make clean
```

## Proje Yapısı

```
RetaOS/
├── boot.s          # Bootstrap kodu (Multiboot)
├── kernel.c        # Ana kernel kodu
├── linker.ld       # Linker script
├── Makefile        # Build sistemi
├── run.sh          # Çalıştırma scripti
└── build/          # Derleme çıktıları
    ├── kernel.bin  # Binary kernel
    ├── kernel.elf  # ELF kernel
    └── retaos.iso  # Boot edilebilir ISO
```

## Geliştirme

### Yeni Özellik Ekleme

1. Kernel kodunu `kernel.c` dosyasında düzenleyin
2. `make clean && make all` ile yeniden derleyin
3. `./run.sh gfx` ile test edin

### Debug

Debug modunda çalıştırmak için:
```bash
./run.sh debug
```

Bu mod QEMU'yu debugger ile başlatır (port 1234).

## Katkıda Bulunmak

Katkıda bulunmak istiyorsanız:
1. Fork yapın
2. Feature branch oluşturun
3. Değişikliklerinizi commit edin
4. Pull request gönderin

## Lisans

Bu proje MIT lisansı ile lisanslanmıştır.

## Sorun Giderme

### Derleme Hataları
- Tüm gereksinimlerin yüklü olduğundan emin olun
- `make clean` ile temizlik yapın

### QEMU Çalışmıyor
- QEMU'nun yüklü olduğunu kontrol edin
- ISO dosyasının oluşturulduğunu kontrol edin

### Ekran Çıktısı Yok
- Grafik modunda çalıştırmayı deneyin: `./run.sh gfx`
- Seri konsol modunu deneyin: `./run.sh serial`

---