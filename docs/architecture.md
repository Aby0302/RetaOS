# RetaOS Mimarisi: Genel Bakış

Bu belge, RetaOS’un önyüklemeden (boot) sürücülere kadar ana bileşenlerini yüksek seviyede açıklar.

## 1. Önyükleme (Boot)
- GRUB, `multiboot` spesifikasyonu ile `kernel.elf`’i yükler ve giriş noktasına atlar.
- Başlangıçta temel bellek haritası ve multiboot bilgileri çekirdeğe aktarılır.

## 2. Çekirdek Başlangıcı
- Erken aşamada GDT yüklenir, segmentler tanımlanır.
- IDT kurulur, istisna ve kesme handler’ları kaydedilir.
- Sayfalama (paging) etkinleştirilir; fiziksel çerçeve tahsiscisi ve basit heap başlatılır.

## 3. Kesme Altyapısı
- PIC tabanlı kesmeler (şimdiki yol) yapılandırılır.
- ISR/IRQ şablonları ve `isr.c` handler’ları ile hata raporlama ve `panic()` mekanizması.
- Planlanan: ACPI keşfi → LAPIC/IOAPIC/HPET ile modern kesme altyapısı.

## 4. Zamanlayıcı ve Zamanlama
- PIT ile temel tick üretimi, round-robin (RR) preemptive zamanlayıcı.
- `quantum` ayarlanabilir, `rr on/off` ile preemption kontrolü.
- Planlanan: HPET ile yüksek çözünürlüklü zamanlama ve modüler zamanlayıcı soyutlaması.

## 5. Sürücüler ve G/Ç
- Klavye: scancode → ASCII, halka tampon, satır içi düzenleme için genişletilmiş API.
- Terminal: VGA metin modu, kaydırma ve `kprintf` benzeri biçimlendirme.
- Depolama: ATA/ATAPI okuma, blok cihaz soyutlaması.

## 6. Dosya Sistemi ve VFS
- `initrd.tar` (ustar) okunur, bellek içi VFS ağaç yapısı kurulur.
- Dizin/dosya düğümleri, basit path çözümleme, `ls` ve `cat` komutları.
- Planlanan: API genişlemesi (handle tabanlı open/read/close), FAT12/16 okuma.

## 7. Kullanıcı Alanı ve Syscall’lar (Plan)
- Kısa vadede: Syscall ABI tasarımı ve ring3’e geçiş için iret çerçevesi hazırlığı.
- Orta/uzun vadede: Kullanıcı süreç başlatma (ELF yükleme, stack kurulum, argc/argv), basit libc.

## 8. Debug ve Test
- GDB ile uzak hata ayıklama (QEMU `-s -S`), önerilen breakpoint noktaları.
- CI’de headless QEMU ile smoke test (boot stabilitesini doğrulama).

Daha ayrıntı için:
- `docs/syscalls.md`
- `docs/user-mode.md`
