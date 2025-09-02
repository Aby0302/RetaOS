# RetaOS Kullanıcı Modu (Taslak)

Bu belge, çekirdekten kullanıcı moduna (ring3) geçiş ve kullanıcı süreçlerinin başlatılması için planı açıklar.

## Hedefler
- Ring0 → Ring3 geçişi (IRET çerçevesi ile) ve güvenli dönüş.
- Basit bir kullanıcı süreç yaşam döngüsü: ELF yükleme, stack kurulumu, giriş noktası çağrısı.
- Temel syscall’lar ile çekirdek hizmetlerine erişim.

## Minimum Yol Haritası
1. IRET için gerekli segment ve yığın hazırlığı (TSS gereksinimleri dahil not düşülecek).
2. Test amaçlı bir kullanıcı buffer’ı ve dummy kodu (şimdilik ELF’siz) ile ring3’e atlama.
3. Syscall mekanizması (`int 0x80`) ile ring3 → ring0 çağrısı ve dönüş.
4. Basit ELF yükleyici (32-bit) ve process yapısı iskeleti.

## Teknik Notlar
- GDT’de user code/data segmentleri (DPL=3) eklenmeli.
- TSS ve ayrı ring3 stack’i yapılandırılmalı.
- IRET çerçevesi: EIP, CS, EFLAGS, ESP, SS değerleri kullanıcı segmentleri ile yüklenmeli.
- İlk sürümde `int 0x80`, gelecekte `sysenter` değerlendirilebilir.

## İlgili Belgeler
- `docs/syscalls.md`
- `docs/architecture.md`
