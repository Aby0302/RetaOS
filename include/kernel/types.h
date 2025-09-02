#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H

#include <stddef.h>  // size_t için
#include <stdint.h>  // intX_t ve uintX_t tipleri için

// İşlem ve görev kimlikleri için tipler
typedef int32_t pid_t;
typedef int32_t tid_t;

// Sayfalama ile ilgili tipler
typedef uint32_t* page_directory_t;
typedef uint32_t* page_table_t;

// Hata kodları
typedef int32_t status_t;

// Bayrak tipleri
typedef uint32_t flags_t;

// Fiziksel ve sanal adres tipleri
typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;

// Kullanıcı modu bağlam yapısı (task.h'da tanımlı)
struct user_context;

typedef struct user_context user_context_t;

#endif // _KERNEL_TYPES_H
