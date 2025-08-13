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
  - [ ] Multiboot bellek haritasını okuma ve doğrulama
  - [ ] Fiziksel bellek yöneticisi (frame allocator)
  - [ ] Sanal bellek/paging etkinleştirme (PDE/PTE) ve CR3 yönetimi
  - [ ] Basit kernel heap (`kmalloc`/`kfree`)
  - [ ] Page Fault detaylı raporlama ve kurtarma stratejisi
- Kesme/İstisna
  - [ ] Tüm CPU istisnaları için açıklayıcı handler’lar
  - [ ] Ortak `panic()` altyapısı ve hata ekranı (stack dump, kayıtlar)
- Zamanlayıcı ve Görevler
  - [ ] TSS kurulumu ve bağlam değiştirme (context switch)
  - [ ] Basit zamanlayıcı (Round-Robin) ile çoklu görev
- Sürücüler ve G/Ç
  - [ ] Klavye sürücüsü: scancode → ASCII dönüştürme, halka tampon
  - [ ] VGA terminal: kaydırma, renkler, `printf` benzeri biçimlendirme
  - [ ] Temel depolama (ATA/ATAPI) okuma ve blok cihaz soyutlaması
  - [ ] APIC/HPET ve ACPI tabanlı başlatma (orta/uzun vade)
- Dosya Sistemi
  - [ ] Basit initrd/tarfs yükleyici
  - [ ] VFS taslağı ve path çözümleme
- Sistem Çağrıları ve Kullanıcı Modu
  - [ ] Syscall giriş/çıkış ABI’si
  - [ ] Kullanıcı modu süreç başlatma altyapısı (uzun vadeli)
- Araçlar, Build ve Test
  - [ ] Çapraz derleyici dokümantasyonu (i686-elf) ve opsiyonel kullanımı
  - [ ] CI scriptleri ve otomatik QEMU testleri (headless)
  - [ ] Gelişmiş debug rehberi (GDB betikleri, breakpoint önerileri)
- Dokümantasyon
  - [ ] Mimarinin genel resmi (boot → kernel → kesmeler → sürücüler)
  - [ ] Kod yapısı, katkı rehberi ve tarz kuralları

- Önceliklendirme (ilk hedefler)
  - [ ] Bellek yönetimi (MMU/paging + frame allocator)
  - [ ] İstisna handler’ları ve `panic()`
  - [ ] Terminal geliştirmeleri ve klavye girişi

---