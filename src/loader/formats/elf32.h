#ifndef __ELF32_H__
#define __ELF32_H__

#include <stdint.h>
#include <stddef.h>

// Reference: https://wiki.osdev.org/ELF_Tutorial

// ELF 32-bit Data Tpyes
typedef uint32_t elf32_addr_t;  // Unsigned program address
typedef uint16_t elf32_half_t;  // Unsigned medium integer
typedef uint32_t elf32_off_t;   // Unsigned file offset
typedef int32_t  elf32_sword_t; // Signed word
typedef uint32_t elf32_word_t;  // Unsigned word

/* ELF Header */
#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

// ELF Identification Indexes
typedef enum {
    EI_MAG0       = 0, // File identification
    EI_MAG1       = 1, // File identification
    EI_MAG2       = 2, // File identification
    EI_MAG3       = 3, // File identification
    EI_CLASS      = 4, // File class
    EI_DATA       = 5, // Data encoding
    EI_VERSION    = 6, // File version
    EI_OSABI      = 7, // OS/ABI identification
    EI_ABIVERSION = 8, // ABI version
    EI_PAD        = 9, // Start of padding bytes
    EI_NIDENT     = 16 // Size of e_ident[]
} elf32_ident_indexes_t;

// ELF File Classes
enum {
    ELFCLASSNONE = 0, // Invalid class
    ELFCLASS32   = 1, // 32-bit objects
    ELFCLASS64   = 2  // 64-bit objects
};

// ELF Data Encodings
enum {
    ELFDATANONE = 0, // Invalid data encoding
    ELFDATA2LSB = 1, // Little-endian
    ELFDATA2MSB = 2  // Big-endian
};

// ELF Object File Types
typedef enum {
    ET_NONE   = 0, // No file type
    ET_REL    = 1, // Relocatable file
    ET_EXEC   = 2, // Executable file
    ET_DYN    = 3, // Shared object file
    ET_CORE   = 4, // Core file
    ET_LOPROC = 0xFF00, // Processor-specific
    ET_HIPROC = 0xFFFF  // Processor-specific
} elf32_type_t;

// ELF Header Structure
typedef struct {
    unsigned char e_ident[EI_NIDENT]; // ELF identification
    elf32_half_t  e_type;              // Object file type
    elf32_half_t  e_machine;           // Machine type
    elf32_word_t  e_version;           // Object file version
    elf32_addr_t  e_entry;             // Entry point address
    elf32_off_t   e_phoff;             // Program header offset
    elf32_off_t   e_shoff;             // Section header offset
    elf32_word_t  e_flags;             // Processor-specific flags
    elf32_half_t  e_ehsize;            // ELF header size
    elf32_half_t  e_phentsize;         // Size of program header entry
    elf32_half_t  e_phnum;             // Number of program header entries
    elf32_half_t  e_shentsize;         // Size of section header entry
    elf32_half_t  e_shnum;             // Number of section header entries
    elf32_half_t  e_shstrndx;          // Section name string table index
} __attribute__((packed)) elf32_header_t;

/* ELF Program Header */
// ELF Segment Flags, e.g., p_flags
#define PF_X 0x1 // Execute permission
#define PF_W 0x2 // Write permission
#define PF_R 0x4 // Read permission

// ELF Segment Types
typedef enum {
    PT_NULL    = 0, // Unused segment
    PT_LOAD    = 1, // Loadable segment
    PT_DYNAMIC = 2, // Dynamic linking information
    PT_INTERP  = 3, // Interpreter information
    PT_NOTE    = 4, // Auxiliary information
    PT_SHLIB   = 5, // Reserved
    PT_PHDR    = 6, // Program header table
    PT_LOPROC  = 0x70000000, // Processor-specific
    PT_HIPROC  = 0x7FFFFFFF  // Processor-specific
} elf32_segment_type_t;

// ELF Program Header Structure
typedef struct {
    elf32_word_t p_type;   // Segment type
    elf32_off_t  p_offset; // Segment file offset
    elf32_addr_t p_vaddr;  // Segment virtual address
    elf32_addr_t p_paddr;  // Segment physical address
    elf32_word_t p_filesz; // Segment size in file
    elf32_word_t p_memsz;  // Segment size in memory
    elf32_word_t p_flags;  // Segment flags
    elf32_word_t p_align;  // Segment alignment
} __attribute__((packed)) elf32_program_header_t;

void* elf32_get_entry_pointer(elf32_header_t* elf_header);
uint32_t elf32_get_entry(elf32_header_t* elf_header);

#endif // __ELF32_H__