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
├── linker.ld         # Linker script
├── Makefile          # Build sistemi
├── run.sh            # Çalıştırma scripti
├── src/
│   ├── kernel/
│   │   └── kernel.c                # Ana kernel kodu
│   ├── arch/
│   │   └── x86/
│   │       ├── boot/
│   │       │   ├── boot.s          # Multiboot v1 giriş (ENTRY=_start)
│   │       │   └── boot2.S         # Multiboot v2 giriş (opsiyonel)
│   │       ├── gdt/
│   │       │   ├── gdt.c           # GDT kurulum
│   │       │   └── gdt_flush.S     # GDT yükleme (lgdt, segment yükleme)
│   │       └── interrupts/
│   │           ├── idt.c           # IDT kurulum ve IRQ dispatcher
│   │           ├── idt_load.S      # IDT yükleme (lidt)
│   │           └── isr.S           # ISR/IRQ stub’ları
│   └── drivers/
│       └── serial/
│           └── serial.c            # COM1 sürücüsü
└── build/            # Derleme çıktıları
    ├── kernel.bin    # Binary kernel
    ├── kernel.elf    # ELF kernel
    └── retaos.iso    # Boot edilebilir ISO
```

## Geliştirme

### Yeni Özellik Ekleme

1. İlgili modül altında kodu düzenleyin (örn. `src/kernel/kernel.c`, `src/arch/x86/interrupts/idt.c`)
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

## Kod Yapısı ve Modüller

- `src/arch/x86/boot/`: Multiboot giriş kodları
- `src/arch/x86/gdt/`: GDT/TSS kurulumu ve `lgdt/ltr` yükleme
- `src/arch/x86/interrupts/`: IDT, ISR/IRQ stub’ları ve dispatcher
- `src/kernel/`: Çekirdek ana akışı (`kernel.c`), zamanlayıcı (`sched.c`)
- `src/drivers/serial/`, `src/drivers/keyboard/`: Temel G/Ç sürücüleri
- `src/memory/`: PMM/heap, paging (ileride genişletilecek)
- `include/`: Başlık dosyaları ve ortak arayüzler

## Katkı Rehberi ve Kod Tarzı

- C standardı: freestanding C (C11 özellikleri sınırlı), `-ffreestanding -fno-builtin`
- Stil: 2 veya 4 boşluk tutarlı kullanın; başlık/impl ayrımı; dosya üstünde kısa özet.
- Commit mesajları: kısa özet (50 karakter), gerekirse detaylı açıklama.
- PR’lerde: test adımları, QEMU çıktısı veya GDB ekran görüntüsü ekleyin.

## Çapraz Derleyici (i686-elf) Kurulumu

Yerel derleyicinizle de derlenebilir; ancak tutarlılık için i686-elf toolchain önerilir.

Özet kurulum (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget

# Kaynakları alın (önek /opt/cross, hedef i686-elf)
export PREFIX=/opt/cross
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
tar -xf binutils-2.40.tar.xz && tar -xf gcc-13.2.0.tar.xz

mkdir -p build-binutils && cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc) && sudo make install
cd ..

mkdir -p build-gcc && cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc) && sudo make install-gcc
make all-target-libgcc -j$(nproc) && sudo make install-target-libgcc
```

Kullanım:
```bash
export PATH=/opt/cross/bin:$PATH
make CROSS=1
```
Makefile `CROSS=1` ile `i686-elf-gcc` ve `i686-elf-ld` kullanacak şekilde yapılandırılabilir.

## Gelişmiş Debug Rehberi (GDB)

QEMU’yu GDB ile başlatma:
```bash
./run.sh debug
# Ayrı bir terminalde:
gdb -q build/kernel.elf \
  -ex 'set architecture i386' \
  -ex 'target remote :1234' \
  -ex 'break kernel_main' \
  -ex 'continue'
```
Faydalı breakpoint önerileri: `exception_handler`, `irq_handler`, `scheduler_start`, `context_switch`.

## CI ve Headless QEMU Testleri

Başlangıç için yerel smoke test:
```bash
qemu-system-i386 -cdrom build/retaos.iso -display none -serial stdio -no-reboot -no-shutdown \
  -d int,cpu_reset -D build/qemu.log -m 64
```
GitHub Actions örneği (özet):
```yaml
name: build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: deps
        run: sudo apt-get update && sudo apt-get install -y gcc-multilib nasm xorriso mtools qemu-system-x86 grub-pc-bin
      - name: build
        run: make all
      - name: smoke
        run: qemu-system-i386 -cdrom build/retaos.iso -display none -serial stdio -no-reboot -no-shutdown -m 64 -smp 1 -cpu qemu32 -d cpu_reset -D build/qemu.log & sleep 5; kill %1 || true
```

## Mevcut Özellikler (Teknik)

- Boot ve Çekirdek
  - Multiboot uyumlu önyükleme (GRUB)
  - 32-bit protected mode; GDT ile kod/veri segmentleri (gdt.c, gdt_flush.S)
- Kesme Altyapısı
  - 256 girişli IDT kurulumu (idt.c, idt_load.S)
  - PIC yeniden eşleme (0x20/0x28) ve EOI yönetimi
  - IRQ0: PIT 100 Hz zamanlayıcı; IRQ1: klavye kesmesi
  - Temel sayfa hatası (Page Fault, ISR14) yakalama kancası
  - `irq_install_handler`, `interrupts_enable/disable` yardımcıları
- Zamanlayıcı ve G/Ç
  - PIT 100 Hz tick sayacı; her 100 tick’te seri porta log
  - VGA metin modu (80x25) terminal: basit yazı, satır sonu, ekran temizleme
  - Seri port (COM1, 115200): metin/hex/decimal yazma yardımcıları
- Build ve Çalıştırma
  - ISO üretimi (Makefile, `linker.ld`)
  - QEMU ile grafik/seri modda çalışma; debug modu (GDB, port 1234)

## Planlanan Özellikler ve Yapılacaklar

- Bellek Yönetimi
  - [x] Multiboot bellek haritasını okuma ve doğrulama
  - [x] Fiziksel bellek yöneticisi (frame allocator)
  - [x] Sanal bellek/paging etkinleştirme (PDE/PTE) ve CR3 yönetimi
  - [x] Basit kernel heap (`kmalloc`/`kfree`)
  - [x] Page Fault detaylı raporlama ve kurtarma stratejisi (panic)
- Kesme/İstisna
  - [x] Page Fault hata kodu ve CR2 ayrıntılı raporlama
  - [x] Ortak `panic()` altyapısı ve hata ekranı (stack dump, kayıtlar)
- Zamanlayıcı ve Görevler
  - [x] TSS kurulumu ve bağlam değiştirme (context switch)
  - [x] Basit zamanlayıcı (Round-Robin) ile çoklu görev

### Zamanlayıcı: Teknik Notlar

- Zaman dilimi (quantum): Varsayılan 5 tick. Çalışma anında `scheduler_set_quantum(n)` ile değiştirilebilir.
- Preemptive yol (ISR çıkışı):
  - `IRQ0` için `isr.S` içinde `irq0_stub` registerları `pushal` ile kaydeder, mevcut ESP’yi C tarafına iletir ve `irq0_handler(esp)` çağırır.
  - `irq0_handler` önce merkezi `irq_handler(0)` (EOI + `timer_tick`) çalıştırır, ardından `scheduler_on_timer_isr(isr_esp)` ile preemption ihtiyacını sorar.
  - `scheduler_on_timer_isr` quantum bitti ise mevcut görevin `isr_esp`’sini kaydeder, sıradaki görevi seçer ve hazır bir ISR dönüş çerçevesi varsa yeni görevin ESP’sini döndürür.
  - Assembly stub EAX!=0 ise ESP’yi bu değere geçirir; `popal; iret` ile doğrudan yeni göreve döner.
- Yeni thread başlangıcı: Her yeni görev için yığın üzerinde sahte bir `pushal + iret` çerçevesi hazırlanır (EIP=`thread_boot`, CS=0x08, EFLAGS(IF)=1). Böylece görevler ISR çıkışı yolundan da güvenli başlatılabilir.
- Cooperative yol: Mevcut `yield()`/`scheduler_tick()` akışı korunur. Preemption devredeyken de uyumludur.
- Test:
  - Grafikli: `qemu-system-i386 -cdrom build/retaos.iso`
  - Headless/seri: `qemu-system-i386 -cdrom build/retaos.iso -serial stdio -display none`
  - İki demo thread’in çıktılarının dönüşümlü gelmesi ve periyodik `[tick]` log’ları zaman paylaşımını gösterir. Quantum’u değiştirerek etkisini gözlemleyin.
- Notlar:
  - Kısa bir kritik bölüm (cli/sti) ile yeniden giriş riskini azaltma ileride eklenebilir.
  - Kullanıcı modu eklendiğinde (ring değişimi) iret çerçevesi hazırlığı genişletilecektir.
- Sürücüler ve G/Ç
  - [x] Klavye sürücüsü: scancode → ASCII dönüştürme, halka tampon
    - Shift, CapsLock, ok tuşları (E0), Home/End, Delete desteği eklendi
    - Satır içi düzenleme için genişletilmiş API (özel tuş kodları) sağlandı
  - [x] VGA terminal: kaydırma, renkler, `printf` benzeri biçimlendirme
    - Basit `kprintf`, satır kaydırma, renkli çıktı ve satır içi düzenleme
  - [x] Temel depolama (ATA/ATAPI) okuma ve blok cihaz soyutlaması
  - [ ] APIC/HPET ve ACPI tabanlı başlatma (orta/uzun vade)
    - Amaç: PIC yerine LAPIC/IOAPIC ve yüksek çözünürlüklü HPET ile modern kesme/zamanlayıcı altyapısı
    - ACPI Keşif: RSDP → RSDT/XSDT → MADT (APIC), FADT (HPET adresi), HPET tabloları
    - LAPIC: MSR üzerinden etkinleştirme, spurious vector ayarı, EOI yazma
    - IOAPIC: MMIO tabanlı kayıt erişimi, IRQ → GSI eşleme ve yeniden yönlendirme
    - HPET: MMIO tabanlı sayaç başlatma, periyodik karşılaştırma ve IRQ üretimi
    - Zamanlayıcı Soyutlama: PIT/HPET seçimi; scheduler tick kaynağını modülerleştirme
    - IRQ Yönlendirme: Klavye (IRQ1) ve ATA (IRQ14/15) için IOAPIC girişleri
    - QEMU Testi: `-machine q35`/`-M q35`, `-no-hpet` kapatılmadan HPET doğrulama, LAPIC/IOAPIC açık
    - Geriye Uyum: ACPI bulunamazsa PIT+PIC (mevcut yol) ile çalışmaya devam et
    - Notlar: SMP planlandığında LAPIC IPI/Local Timer genişletilecek
 - Dosya Sistemi
  - [x] Basit initrd/tarfs (ustar) yükleyici
    - hda üzerinde LBA 1’den itibaren arşivi okur (okuma sınırı ~1–2MiB)
    - ustar başlıklarını parse eder, bellek içinde VFS ağaç yapısını kurar
  - [x] VFS taslağı ve path çözümleme
    - Dizin/dosya düğümleri, çocuk listesi, basit path yürütme ve lookup
    - Dosya oku (read-all) ve dizin listeleme arayüzleri
  - [x] Shell entegrasyonu
    - `ls [PATH]` ve `cat PATH` komutları
  - [x] Build ve test kolaylığı
    - Makefile’da `initrd`, `disk`, `run-disk`, `run-gfx-disk` hedefleri
  - Notlar
    - Şimdilik read-only; uzun isimler ve büyük arşivler için iyileştirme planlı
  - Planlanan geliştirmeler
    - Dosya öznitelikleri (boyut, zaman damgası, izinler — temel)
    - Daha sağlam tar yorumlayıcı (uzun isimler, çok sayıda entry)
    - VFS API genişletmesi (open/read/close benzeri handle tabanlı)
    - Farklı dosya sistemleri için sürücü iskelesi (ör. FAT12/16 okuma)
  - Sistem Çağrıları ve Kullanıcı Modu
  - [x] Syscall giriş/çıkış ABI’si (taslak) — bkz. [docs/syscalls.md](docs/syscalls.md)
  - [ ] Kullanıcı modu süreç başlatma altyapısı (taslak, uzun vadeli) — bkz. [docs/user-mode.md](docs/user-mode.md)
 - Araçlar, Build ve Test
  - [x] Çapraz derleyici dokümantasyonu (i686-elf) ve opsiyonel kullanımı
  - [x] CI scriptleri ve otomatik QEMU testleri (headless) — GitHub Actions + QEMU smoke test ([scripts/qemu-smoke.sh](scripts/qemu-smoke.sh))
  - [x] Gelişmiş debug rehberi (GDB betikleri, breakpoint önerileri)
 - Dokümantasyon
  - [x] Mimarinin genel resmi (boot → kernel → kesmeler → sürücüler) — bkz. [docs/architecture.md](docs/architecture.md)
  - [x] Kod yapısı, katkı rehberi ve tarz kuralları

- Önceliklendirme (ilk hedefler)
  - [x] Bellek yönetimi (MMU/paging + frame allocator)
  - [x] İstisna handler’ları ve `panic()` (temel)
  - [x] Terminal geliştirmeleri ve klavye girişi

### Terminal / Shell

Kernelde bir shell thread'i çalışır ve preemptive RR zamanlayıcı ile görev paylaşır. Aşağıdaki özellikler ve komutlar desteklenir:

#### Komutlar (özet)
- `help` — Komut listesini gösterir.
- `ps` — Görev sayısı, aktif görev indeksi, RR quantum ve preemption durumu.
- `quantum N` — Round-Robin quantum'u (tick) ayarla.
- `rr on|off` — Preemption aç/kapat.
- `time` — PIT tick sayacı ve yaklaşık süre.
- `echo TEXT` — Metni ekrana yazdırır.
- `blk` — Kayıtlı blok cihazları listeler.
- `blkread DEV LBA N` — N sektör okur ve hexdump yazar (N≤8 önerilir).
- `part DEV` — MBR birincil bölümleri gösterir.
- `ls [PATH]` — VFS dizin içeriklerini listeler (varsayılan `/`).
- `cat PATH` — VFS dosya içeriğini yazar (şimdilik ~2KB sınırı).
- `clear` — Ekranı temizler.

#### Kısayollar ve düzenleme
- Ctrl+A: Satır başına git
- Ctrl+E: Satır sonuna git
- Ctrl+U: Satırı temizle
- Yukarı/Aşağı: Komut geçmişi (HIST_MAX=16)

#### Örnek oturum
```
Welcome to RetaOS!
Type 'help' for commands.

$ blk
hda 512B/sector

$ part hda
MBR partitions (primary):
  #0 boot=0x00 type=0x83 start=2048 size=...

$ ls
bin/
etc/

$ ls /etc
motd

$ cat /etc/motd
Hello from RetaOS

$ rr on
preemption: on

$ quantum 5

```

- Etkileşimli shell çekirdek thread'i olarak çalışır, preemptive RR zamanlayıcı ile birlikte çoklu görev paylaşır.
- Satır içi düzenleme: Sol/Sağ, Home/End, Backspace, Delete, ekleme modu.
- Kısayollar: Ctrl+A (başa), Ctrl+E (sona), Ctrl+U (satırı temizle).
- Komut geçmişi: Yukarı/Aşağı ile dolaşma (HIST_MAX=16).
- Komutlar:
  - `help` — Yardım ve komut listesi.
  - `ps` — Görev sayısı, mevcut görev indeksi, RR quantum ve preemption durumu.
  - `quantum N` — RR quantum'u (tick) ayarla.
  - `rr on|off` — Preemption (kesmeli zamanlama) aç/kapat.
  - `time` — PIT tick sayacı ve yaklaşık süre (100Hz → 10ms çözünürlük).
  - `echo TEXT` — TEXT çıktısı üretir.
  - `blk` — blok cihazları listele (ör. hda).
  - `blkread DEV LBA N` — N sektör oku ve hexdump göster (N≤8).
  - `part DEV` — DEV üzerinde MBR birincil bölüm tablosunu göster.
  - `ls [PATH]` — VFS üzerinde dizin içeriğini listele (varsayılan `/`).
  - `cat PATH` — VFS üzerinde dosya içeriğini yazdır (şimdilik ~2KB sınırı).
  - `clear` — Terminal ekranını temizler.

Notlar
- PIT 100Hz (10ms) olarak kuruludur; `time` komutu kabaca süre ölçümü sağlar.
- Shell, giriş beklerken `yield()` ile feragat eder; sistem genelinde tepkiselliği artırır.
- Komut geçmişi ve satır düzenleme sabit boyutlu tamponlarla basitçe uygulanmıştır.

Hızlı Depolama Testi (QEMU)
- Boş disk imajı oluşturun: `dd if=/dev/zero of=disk.img bs=1M count=16`
- QEMU ile çalıştırın: `qemu-system-i386 -cdrom build/retaos.iso -m 128M -vga std -hda disk.img`
- Shell'de:
  - `blk` → `hda` listelenmeli.
  - `blkread hda 0 1` → LBA0 sektör hexdump'ı.
  - `part hda` → MBR imzası varsa birincil bölümler listelenir.

### Initrd / VFS Hızlı Test

Initrd, bir `ustar` (tar) arşivini blok cihazdan (hda) LBA 1'den itibaren okuyup bellekte bir VFS ağacına monte eder. Hızlıca denemek için Makefile hedefleri eklendi:

```bash
# ISO üret (gerekirse)
make all

# Basit bir initrd hazırla ve disk.img içine LBA 1'e yaz
make disk

# QEMU ile grafik modda çalıştır (disk.img takılı)
make run-gfx-disk

# veya seri mod
make run-disk
```

`make disk` şunları yapar:
- `initroot/` altına örnek dosyalar (etc/motd, bin/demo.txt) koyar
- `initroot/` içinden `ustar` formatlı `initrd.tar` oluşturur
- `initrd.tar`'ı `disk.img` içine 512 bayt ofsetten (LBA 1) itibaren yazar

QEMU açıldığında shell'de:
- `ls` ve `ls /etc`
- `cat /etc/motd`
- `cat /bin/demo.txt`

Notlar:
- Şimdilik yalnızca okuma desteklenir (read-only VFS/initrd).
- Initrd boyutu üst sınırı yaklaşık 1–2 MiB olacak şekilde kısıtlanmıştır (uygulama tarafında okunacak sektör sayısı sınırlandırılmıştır).
- Uzun dosya adları ve çok büyük dizinler için ileride iyileştirmeler planlanmaktadır.

---