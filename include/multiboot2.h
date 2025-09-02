#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

/* The magic number for the Multiboot2 header. */
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

/* The architecture of the kernel (i386 protected mode). */
#define MULTIBOOT_ARCHITECTURE_I386 0

/* The tag types */
#define MULTIBOOT_HEADER_TAG_END 0

/* The Multiboot header. */
struct multiboot_header
{
  /* Must be MULTIBOOT2_HEADER_MAGIC - see above. */
  uint32_t magic;
  
  /* ISA */
  uint32_t architecture;
  
  /* Total header length. */
  uint32_t header_length;
  
  /* The above fields plus this one must equal 0 mod 2^32. */
  uint32_t checksum;
};

#endif /* ! MULTIBOOT2_H */
