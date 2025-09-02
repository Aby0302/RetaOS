# RetaOS Syscall ABI Taslağı

Bu belge, RetaOS için sistem çağrıları (syscall) giriş/çıkış sözleşmesine dair bir taslaktır.

## Hedefler
- Basit ve mimariye uygun arayüz (x86, i386).
- IRQ/exception çerçevesiyle uyumlu geçiş ve dönüş kuralları.
- Gelecekte kullanıcı modu süreçleri ve basit libc ile entegrasyon.

## Çağrı Mekanizması Seçenekleri
- `int 0x80`: Basit, yaygın yöntem.
- `sysenter`/`sysexit`: Daha hızlı, fakat ek MSR kurulumu gerekir (ileride değerlendirilecek).
- İlk sürüm için `int 0x80` önerilir.

## Register ABI (öneri)
- `eax`: Syscall numarası
- `ebx`, `ecx`, `edx`, `esi`, `edi`: Argüman taşıma kayıtları (maks 5 argüman)
- Dönüş değeri: `eax`
- Hata kodu: negatif değerler (örn. `-1`, `-EINVAL` vb.)

## Çekirdek Tarafı
- IDT’de 0x80 vektörü için handler kaydı.
- `isr.c` içinde kullanıcı/çekirdek ring ayrımı gözetilerek uygun iret çerçevesi ile dönüş.
- Reentrancy/Preemption: Kısa kritik bölüm veya scheduler ile entegrasyon notları.

## Kullanıcı Tarafı (ileride)
- `libc` benzeri bir sarmalayıcı: `long syscall(n, a1, a2, ...)`.
- Inline asm ile `int $0x80` çağrısı ve register düzenlemesi.

## Örnek (çekirdek pseudo-code)
```
void isr_syscall(Regs* r) {
    uint32_t num = r->eax;
    uint32_t a1 = r->ebx, a2 = r->ecx, a3 = r->edx, a4 = r->esi, a5 = r->edi;
    r->eax = syscall_dispatch(num, a1, a2, a3, a4, a5);
}
```

## Güvenlik ve Validasyon
- Kullanıcı bellek erişimlerinde kopyalama/validasyon (kısmi plan).
- Ring geçişi: IRET çerçevesi ile güvenli dönüş.

## Yol Haritası
1. `int 0x80` tabanlı iskelet ve dummy syscall (örn. `sys_write` → terminale yazma).
2. Basit numara tablosu ve dispatcher.
3. Gelecekte: `sysenter` geçişi ve performans iyileştirmeleri.
