---

# RetaOS

RetaOS, modern ve modüler bir işletim sistemi projesidir. Temel amacı, açık kaynaklı ve öğrenmesi kolay bir işletim sistemi mimarisi sunmaktır.

## Özellikler

- **Minimal Kernel:** Basit ve anlaşılır çekirdek mimarisi
- **Multiboot Uyumlu:** GRUB ile uyumlu boot sistemi
- **VGA Terminal:** Basit terminal çıktısı
- **QEMU Desteği:** Sanal makinede çalıştırma
- **Kolay Kurulum:** Hızlı ve pratik kurulum desteği
- **Çoklu İşlem Desteği:** Kullanıcı modunda çalışan birden fazla program
- **Gelişmiş Shell:** Temel komutları destekleyen kabuk uygulaması
- **Standart C Kütüphanesi:** Libc uyumlu kütüphane desteği

## Gereksinimler

Aşağıdaki paketlerin yüklü olması gerekiyor:

```bash
sudo apt install -y gcc-multilib build-essential nasm xorriso mtools qemu-system-x86 grub-pc-bin
```

## Kullanıcı Programları

RetaOS, aşağıdaki temel kullanıcı programlarını içerir:

### 1. init

Sistem başlangıç programı. Varsayılan olarak shell'i başlatır. Eğer shell başlatılamazsa, otomatik olarak `hello` programını çalıştırır.

### 2. shell

Temel bir kabuk uygulaması. Aşağıdaki komutları destekler:

- `help`: Kullanılabilir komutları listeler
- `echo [metin]`: Girilen metni ekrana yazdırır
- `clear`: Ekranı temizler
- `ps`: Çalışan işlemleri listeler
- `meminfo`: Bellek kullanım bilgilerini gösterir
- `exit`: Shell'den çıkar

### 3. hello

Basit bir test programı. Sistem bilgilerini gösterir ve 3 saniye sonra kapanır.

## Kurulum ve Çalıştırma

### 1. Projeyi Derleme

```bash
# Tüm projeyi derle
make all

# Sadece kullanıcı programlarını derle
make -C user
```

Bu komutlar:
- Kernel'i ve kullanıcı programlarını derler
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
# Method 1: Using the run script
./run.sh debug

# Method 2: Using the dedicated debug script
./debug.sh

# Method 3: Using make
make debug
```

### Debugging

RetaOS supports debugging with GDB. Here's how to use it:

#### 1. Start QEMU in Debug Mode

```bash
# Option 1: Use the dedicated debug script (recommended)
./debug.sh

# Option 2: Use the run script
./run.sh debug

# Option 3: Use make
make debug
```

#### 2. Connect with GDB

In another terminal, connect to the debugger:

```bash
# Option 1: Use the provided GDB script (recommended)
gdb -x debug.gdb

# Option 2: Manual connection
gdb -ex 'target remote localhost:1234' -ex 'symbol-file build/kernel.elf'
```

#### 3. Debugging Commands

Once connected, you can use standard GDB commands:

```bash
(gdb) continue          # Continue execution
(gdb) step              # Step into function
(gdb) next              # Step over function
(gdb) break kernel_main # Set breakpoint
(gdb) info registers    # Show CPU registers
(gdb) bt                # Show backtrace
(gdb) x/10i $eip       # Show 10 instructions at current position
```

#### 4. Useful Breakpoints

```bash
(gdb) break kernel_main  # Break at kernel entry point
(gdb) break kmain        # Break at main kernel function
(gdb) break shell_main   # Break at shell entry point
```

#### 5. Debug Logs

All debug sessions are automatically logged to the `logs/` directory:

```bash
# View latest debug log
ls -la logs/
cat logs/debug_YYYYMMDD_HHMMSS.log

# View all debug logs
find logs/ -name "debug_*.log" -exec cat {} \;
```

The log files contain:
- Timestamp of each operation
- QEMU startup information
- Error messages and warnings
- Debug session details

### 3. Temizlik

```bash
make clean
```

## Proje Yapısı

```
RetaOS/
├── Makefile          # Build sistemi
├── run.sh            # Çalıştırma scripti
├── debug.sh          # Debug scripti
├── arch/
│   └── x86/
│       └── x86/      # Mimariye özel kodlar
├── boot/
│   ├── boot2.S       # Bootloader kodu
│   └── linker.ld     # Linker scripti
├── build/            # Derleme çıktıları
│   ├── retaos.iso    # Boot edilebilir ISO
│   ├── kernel.elf    # ELF kernel
│   └── disk.img      # Disk imajı
├── docs/             # Dokümantasyon
│   ├── architecture.md
│   ├── syscalls.md
│   └── user-mode.md
├── drivers/          # Sürücüler
│   ├── ata/
│   ├── input/
│   ├── keyboard/
│   ├── network/
│   ├── serial/
│   └── sound/
├── fs/               # Dosya sistemi
│   ├── fat32.c
│   ├── fat32.h
│   ├── fat32_impl.c
│   ├── fat32_new.c
│   ├── fat32_vfs.c
│   └── fat32_vfs.h
├── include/          # Başlık dosyaları
│   ├── errno.h
│   ├── multiboot.h
│   ├── drivers/
│   ├── gui/
│   ├── kernel/
│   ├── memory/
│   └── sys/
├── initroot/         # Başlangıç dosyaları
│   ├── bin/
│   ├── etc/
├── kernel/           # Çekirdek kodları
│   ├── block.c
│   ├── bsod.c
│   ├── console.c
│   ├── debug.c
│   ├── display.c
│   ├── elf.c
│   ├── init.c
│   ├── irq.c
│   ├── kernel.c
│   ├── kheap.c
│   ├── sched.c
│   ├── shell.c
│   ├── splash.c
│   ├── string.c
│   ├── syscalls.c
│   ├── task.c
│   ├── thread.c
│   ├── timer.c
│   └── vfs.c
├── libc/             # Standart C kütüphanesi
│   ├── Makefile
│   ├── crt/
│   ├── include/
│   └── src/
├── logs/             # Log dosyaları
├── memory/           # Bellek yönetimi
├── scripts/          # Yardımcı scriptler
├── user/             # Kullanıcı programları
│   ├── Makefile
│   ├── crt/
│   ├── gui/
│   └── sh/
└── gui/              # Grafik arayüz kodları
```

## Geliştirme

### Yeni Özellik Ekleme

1. İlgili modül altında kodu düzenleyin (örn. `kernel/kernel.c`, `drivers/serial/serial.c`)
2. `make clean && make all` ile yeniden derleyin
3. `make run-gfx` veya `./run.sh gfx` ile test edin

### Debug

Debug modunda çalıştırmak için:
```bash
./run.sh debug
```

Bu mod QEMU'yu debugger ile başlatır (port 1234).

## Mimari Genel Resim

Boot → Kernel → Kesmeler → Sürücüler akışı kısaca:
- GRUB, Multiboot bilgileri ile `kernel.elf`’i yükler ve `_start`’a aktarır.
- GDT yüklenir (kod/veri segmentleri, TSS); IDT hazırlanır ve PIC yeniden eşlenir.
- Temel bellek yönetimi başlatılır: Multiboot bellek haritası okunur, fiziksel frame tahsisçisi kurulur; paging etkinleşir; kernel heap (`kmalloc/kfree`) hazırlanır.
- ISR/IRQ kancaları: CPU istisnaları ve donanım kesmeleri (PIT/keyboard) bağlanır.
- Sürücüler (seri, klavye, VGA) başlatılır; terminal üzerinden çıktı sağlanır.
- Basit görev zamanlayıcı iskeleti çalışır; ileride preemptive RR ve kullanıcı modu eklenecektir.

## Katkı Rehberi ve Kod Tarzı

- C standardı: freestanding C (C11 özellikleri sınırlı), `-ffreestanding -fno-builtin`
- Stil: 2 veya 4 boşluk tutarlı kullanın; başlık/impl ayrımı; dosya üstünde kısa özet.
- Commit mesajları: kısa özet (50 karakter), gerekirse detaylı açıklama.
- PR’lerde: test adımları, QEMU çıktısı veya GDB ekran görüntüsü ekleyin.

---