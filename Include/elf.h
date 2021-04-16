/**
 * Created by TekuConcept on February 18, 2020
 */

#ifndef ELF_H
#define ELF_H

#include <iostream>

typedef unsigned int Elf32_Addr; // 4 bytes - program address
typedef unsigned short Elf32_Half; // 2 bytes - medium integer
typedef unsigned int Elf32_Off; // 4 bytes - file offset
typedef signed int Elf32_Sword; // 4 bytes - large integer
typedef unsigned int Elf32_Word; // 4 bytes - large integer
// unsigned char - 1 byte - small integer

enum elf_type {
    ET_NONE   = 0,      // No file type
    ET_REL    = 1,      // Relocatable file
    ET_EXEC   = 2,      // Executable file
    ET_DYN    = 3,      // Shared object file
    ET_CORE   = 4,      // Core file
    ET_LOPROC = 0xFF00, // Processor-specific
    ET_HIPROC = 0xFFFF  // Processor-specific
};

enum elf_machine {
    EM_NONE  = 0,       // No machine
    EM_M32   = 1,       // AT&T WE 32100
    EM_SPARC = 2,       // SPARC
    EM_386   = 3,       // Intel 80386
    EM_68K   = 4,       // Motorola 68000
    EM_88K   = 5,       // Motorola 88000
    EM_860   = 7,       // Intel 80860
    EM_MIPS  = 8,       // MIPS RS3000
    EM_ARM   = 40       // ARM
};

enum elf_version {
    EV_NONE = 0,        // Invalid version
    EV_CURRENT = 1      // Current version
};

// Identification indices
enum elf_ident_idx {
    EI_MAG0    = 0,     // File identification
    EI_MAG1    = 1,     // File identification
    EI_MAG2    = 2,     // File identification
    EI_MAG3    = 3,     // File identification
    EI_CLASS   = 4,     // File class
    EI_DATA    = 5,     // Data encoding
    EI_VERSION = 6,     // File version
    EI_PAD     = 7,     // Start of padding bytes
    EI_NIDENT  = 16     // Size of e_ident[]
};

enum elf_shn_idx {
    // This value marks an undefined, missing, irrelevant, or
    // otherwise meaningless section reference. For example, a symbol
    // "defined" relative to section number SHN_UNDEF is an undefined
    // symbol.
    SHN_UNDEF     = 0,
    // This value specifies the lower bound of the range of reserved
    // indices.
    SHN_LORESERVE = 0xFF00,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    SHN_LOPROC    = 0xFF00,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    SHN_HIPROC    = 0xFF1F,
    // This value specifies absolute values for the corresponding
    // reference. For example, symbols defined relative to section
    // number SHN_ABS have absolute values and are not affected by
    // relocation.
    SHN_ABS       = 0xFFF1,
    // Symbols defined relative to this section are common symbols,
    // such as FORTRAN COMMON or unallocated C external variables.
    SHN_COMMON    = 0xFFF2,
    // This value specifies the upper bound of the range of reserved
    // indices. The system reserved indices between SHN_LORESERVE and
    // SHN_HIRESERVE, inclusive; the values do not reference the
    // section header table. That is, the section header table does
    // not contain entries for the reserved indices.
    SHN_HIRESERVE = 0xFFFF
};

enum elf_sh_type {
    // This value marks the section header as inactive; it does not
    // have an associated section. Other members of the section
    // header have undefined values
    SHT_NULL               = 0,
    // The section holds information defined by the program, whose
    // format and meaning are determined solely by the program.
    SHT_PROGBITS           = 1,
    // These sections hold a symbol table. Currently, an object file
    // may have only one section of each type, but this restriction
    // may be relaxed in the future. Typically, SHT_SYMTAB provides
    // symbols for link editing, though it may also be used for
    // dynamic linking. As a complete symbol table, it may contain
    // many symbols unnecessary for dynamic linking. Consequently,
    // an object file may also contain a SHT_DYNSYM section, which
    // holds a minimal set of dynamic linking symbols, to save space.
    SHT_SYMTAB             = 2,
    // The section holds a string table. An object file may have
    // multiple string table sections.
    SHT_STRTAB             = 3,
    // The section holds relocation entries with explicit addends,
    // such as type Elf32_Rela for the 32-bit class of object files.
    // An object file may have multiple relocation sections
    SHT_RELA               = 4,
    // The section holds a symbol hash table. All objects
    // participating in dynamic linking must contain a symbol hash
    // table. Currently, an object file may have only one hash table,
    // but this restriction may be relaxed in the future. 
    SHT_HASH               = 5,
    // The section holds information for dynamic linking. Currently,
    // an object file may have only one dynamic section, but this
    // restriction may be relaxed in the future. 
    SHT_DYNAMIC            = 6,
    // The section holds information that marks the file in some way.
    SHT_NOTE               = 7,
    // A section of this type occupies no space in the file but
    // otherwise resembles SHT_PROGBITS. Although this section
    // contains no bytes, the sh_offset member contains the
    // conceptual file offset.
    SHT_NOBITS             = 8,
    // The section holds relocation entries without explicit addends,
    // such as type Elf32_Rel for the 32-bit class of object files.
    // An object file may have multiple relocation sections. 
    SHT_REL                = 9,
    // This section type is reserved but has unspecified semantics.
    // Programs that contain a section of this type do not conform to
    // the ABI.
    SHT_SHLIB              = 10,
    SHT_DYNSYM             = 11,
    // Values in this inclusive range are reserved for
    // processor-specific semantics
    SHT_LOPROC             = 0x70000000,
    SHT_ARM_EXIDX          = 0x70000001,
    SHT_ARM_PREEMPTMAP     = 0x70000002,
    SHT_ARM_ATTRIBUTES     = 0x70000003,
    SHT_ARM_DEBUGOVERLAY   = 0x70000004,
    SHT_ARM_OVERLAYSECTION = 0x70000005,
    // Values in this inclusive range are reserved for
    // processor-specific semantics
    SHT_HIPROC             = 0x7FFFFFFF,
    // This value specifies the lower bound of the range of indexes
    // reserved for application programs.
    SHT_LOUSER             = 0x80000000,
    // This value specifies the upper bound of the range of indexes
    // reserved for application programs. Section types between
    // SHT_LOUSER and SHT_HIUSER may be used by the application,
    // without conflicting with current or future system-defined
    // section types.
    SHT_HIUSER             = 0xFFFFFFFF
};

// section attribute flags
enum elf_sh_flags_idx {
    // The section contains data that should be writable during
    // process execution.
    SHF_WRITE            = 1,
    // The section occupies memory during process execution. Some
    // control sections do not reside in the memory image of an
    // object file; this attribute is off for those sections.
    SHF_ALLOC            = 2,
    // The section contains executable machine instructions.
    SHF_EXECINSTR        = 4,
    SHF_MERGE            = 0x10,
    SHF_STRINGS	         = 0x20,
    SHF_INFO_LINK        = 0x40,
    SHF_LINK_ORDER       = 0x80,
    SHF_OS_NONCONFORMING = 0x100,
    SHF_GROUP            = 0x200,
    SHF_TLS              = 0x400,
    SHF_MASKOS           = 0x0FF00000,
    // All bits included in this mask are reserved for
    // processor-specific semantics.
    SHF_MASKPROC         = 0xF0000000
};

enum elf_st_bind {
    // Local symbols are not visible outside the object file
    // containing their definition. Local symbols of the same name
    // may exist in multiple files without interfering with each
    // other.
    STB_LOCAL  = 0,
    // Global symbols are visible to all object files being combined.
    // One file’s definition of a global symbol will satisfy another
    // file’s undefined reference to the same global symbol.
    STB_GLOBAL = 1,
    // Weak symbols resemble global symbols, but their definitions
    // have lower precedence.
    STB_WEAK   = 2,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    STB_LOPROC = 13,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    STB_HIPROC = 15
};

enum elf_st_type {
    // The symbol's type is not specified.
    STT_NOTYPE  = 0,
    // The symbol is associated with a data object, such as a
    // variable, an array, etc.
    STT_OBJECT  = 1,
    // The symbol is associated with a function or other executable
    // code.
    STT_FUNC    = 2,
    // The symbol is associated with a section. Symbol table entries
    // of this type exist primarily for relocation and normally have
    // STB_LOCAL binding.
    STT_SECTION = 3,
    // Conventionally, the symbol’s name gives the name of the source
    // file associated with the object file. A file symbol has
    // STB_LOCAL binding, its section index is SHN_ABS, and it
    // precedes the other STB_LOCAL symbols for the file, if it is
    // present.
    STT_FILE    = 4,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    STT_LOPROC  = 13,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    STT_HIPROC  = 15
};

enum elf_p_type {
    // The array element is unused; other members’ values are
    // undefined. This type lets the program header table have
    // ignored entries.
    PT_NULL    = 0,
    // The array element specifies a loadable segment, described by
    // p_filesz and p_memsz. The bytes from the file are mapped to
    // the beginning of the memory segment. If the segment’s memory
    // size (p_memsz) is larger than the file size (p_filesz), the
    // "extra" bytes are defined to hold the value 0 and to follow
    // the segment’s initialized area. The file size may not be
    // larger than the memory size. Loadable segment entries in the
    // program header table appear in ascending order, sorted on the
    // p_vaddr member.
    PT_LOAD    = 1,
    // The array element specifies dynamic linking information.
    PT_DYNAMIC = 2,
    // The array element specifies the location and size of a
    // null-terminated path name to invoke as an interpreter. This
    // segment type is meaningful only for executable files (though
    // it may occur for shared objects); it may not occur more than
    // once in a file. If it is present, it must precede any loadable
    // segment entry.
    PT_INTERP  = 3,
    // The array element specifies the location and size of auxiliary
    // information.
    PT_NOTE    = 4,
    // This segment type is reserved but has unspecified semantics.
    // Programs that contain an array element of this type do not
    // conform to the ABI.
    PT_SHLIB   = 5,
    // The array element, if present, specifies the location and size
    // of the program header table itself, both in the file and in
    // the memory image of the program. This segment type may not
    // occur more than once in a file. Moreover, it may occur only if
    // the program header table is part of the memory image of the
    // program. If it is present, it must precede any loadable
    // segment entry. 
    PT_PHDR    = 6,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    PT_LOPROC  = 0x70000000,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    PT_HIPROC  = 0x7FFFFFFF
};

enum elf_d_tag {
    // An entry with a DT_NULL tag marks the end of the _DYNAMIC
    // array.
    DT_NULL     = 0,
    // This element holds the string table offset of a
    // null-terminated string, giving the name of a needed library.
    // The offset is an index into the table recorded in the
    // DT_STRTAB entry. The dynamic array may contain multiple
    // entries with this type. These entries’ relative order is
    // significant, though their relation to entries of other types
    // is not.
    DT_NEEDED   = 1,
    // This element holds the total size, in bytes, of the relocation
    // entries associated with the procedure linkage table. If an
    // entry of type DT_JMPREL is present, a DT_PLTRELSZ must
    // accompany it.
    DT_PLTRELSZ = 2,
    // This element holds an address associated with the procedure
    // linkage table and/or the global offset table.
    DT_PLTGOT   = 3,
    // This element holds the address of the symbol hash table. This
    // hash table refers to the symbol table referenced by the
    // DT_SYMTAB element.
    DT_HASH     = 4,
    // This element holds the address of the string table. Symbol
    // names, library names, and other strings reside in this table.
    DT_STRTAB   = 5,
    // This element holds the address of the symbol table with
    // Elf32_Sym entries for the 32-bit class of files.
    DT_SYMTAB   = 6,
    // This element holds the address of a relocation table. Entries
    // in the table have explicit addends, such as Elf32_Rela for the
    // 32-bit file class. An object file may have multiple relocation
    // sections. When building the relocation table for an executable
    // or shared object file, the link editor catenates those
    // sections to form a single table. Although the sections remain
    // independent in the object file, the dynamic linker sees a
    // single table. When the dynamic linker creates the process
    // image for an executable file or adds a shared object to the
    // process image, it reads the relocation table and performs the
    // associated actions. If this element is present, the dynamic
    // structure must also have DT_RELASZ and DT_RELAENT elements.
    // When relocation is "mandatory" for a file, either DT_RELA or
    // DT_REL may occur (both are permitted but not required).
    DT_RELA     = 7,
    // This element holds the total size, in bytes, of the DT_RELA
    // relocation table.
    DT_RELASZ   = 8,
    // This element holds the size, in bytes, of the DT_RELA
    // relocation entry.
    DT_RELAENT  = 9,
    // This element holds the size, in bytes, of the string table.
    DT_STRSZ    = 10,
    // This element holds the size, in bytes, of a symbol table
    // entry.
    DT_SYMENT   = 11,
    // This element holds the address of the initialization function.
    DT_INIT     = 12,
    // This element holds the address of the termination function.
    DT_FINI     = 13,
    // This element holds the string table offset of a
    // null-terminated string, giving the name of the shared object.
    // The offset is an index into the table recorded in the
    // DT_STRTAB entry.
    DT_SONAME   = 14,
    // This element holds the string table offset of a
    // null-terminated search library search path string. The offset
    // is an index into the table recorded in the DT_STRTAB entry.
    DT_RPATH    = 15,
    // This element’s presence in a shared object library alters the
    // dynamic linker’s symbol resolution algorithm for references
    // within the library. Instead of starting a symbol search with
    // the executable file, the dynamic linker starts from the shared
    // object itself. If the shared object fails to supply the
    // referenced symbol, the dynamic linker then searches the
    // executable file and other shared objects as usual.
    DT_SYMBOLIC = 16,
    // This element is similar to DT_RELA, except its table has
    // implicit addends, such as Elf32_Rel for the 32-bit file class.
    // If this element is present, the dynamic structure must also
    // have DT_RELSZ and DT_RELENT elements.
    DT_REL      = 17,
    // This element holds the total size, in bytes, of the DT_REL
    // relocation table.
    DT_RELSZ    = 18,
    // This element holds the size, in bytes, of the DT_REL
    // relocation entry.
    DT_RELENT   = 19,
    // This member specifies the type of relocation entry to which
    // the procedure linkage table refers. The d_val member holds
    // DT_REL or DT_RELA, as appropriate. All relocations in a
    // procedure linkage table must use the same relocation.
    DT_PLTREL   = 20,
    // This member is used for debugging. Its contents are not
    // specified for the ABI; programs that access this entry are
    // not ABI-conforming.
    DT_DEBUG    = 21,
    // This member’s absence signifies that no relocation entry
    // should cause a modification to a non-writable segment, as
    // specified by the segment permissions in the program header
    // table. If this member is present, one or more relocation
    // entries might request modifications to a non-writable segment,
    // and the dynamic linker can prepare accordingly.
    DT_TEXTREL  = 22,
    // If present, this entries’s d_ptr member holds the address of
    // relocation entries associated solely with the procedure
    // linkage table. Separating these relocation entries lets the
    // dynamic linker ignore them during process initialization, if
    // lazy binding is enabled. If this entry is present, the related
    // entries of types DT_PLTRELSZ and DT_PLTREL must also be
    // present.
    DT_JMPREL   = 23,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    DT_LOPROC   = 0x70000000,
    // Values in this inclusive range are reserved for
    // processor-specific semantics.
    DT_HIPROC   = 0x7FFFFFFF
};

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE 0 // Invalid class
#define ELFCLASS32   1 // 32-bit objects
#define ELFCLASS64   2 // 64-bit objects

#define ELFDATANONE 0 // Invalid data encoding
#define ELFDATA2LSB 1 // least significant byte 2's compliment ordering
#define ELFDATA2MSB 2 // most significant byte 2's compliment ordering

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i) & 0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf)

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t)

typedef struct {
    // The initial bytes mark the file as an object file and
    // provide machine-independent data with which to decode
    // and interpret the file's contents. Complete descriptions
    // appear below, in "ELF Identification."
    unsigned char e_ident[EI_NIDENT];
    // This member identifies the object file type.
    // Although the core file contents are unspecified, type ET_CORE
    // is reserved to mark the file. Values from ET_LOPROC through
    // ET_HIPROC (inclusive) are reserved for processor-specific
    // semantics. Other values are reserved and will be assigned to
    // new object file types as necessary.
    Elf32_Half e_type;
    // This member's value specifies the required architecture for
    // an individual file. Other values are reserved and will be
    // assigned to new machines as necessary. Processor-specific ELF
    // names use the machine name to distinguish them. For example,
    // flags use the prefix EF_; a flag names WIDGET for the EM_XYZ
    // machine would be called EF_XYZ_WIDGET.
    Elf32_Half e_machine;
    // This member identifies the object file version. The value 1
    // signifies the original file format; extensions will create
    // new versions with higher numbers. The value of EV_CURRENT,
    // though given as 1, will change as necessary to reflect the
    // current version number.
    Elf32_Word e_version;
    // This member gives the virtual address to which the system
    // first transfers control, thus starting the process. If the
    // file has no associated entry point, this member holds zero.
    Elf32_Addr e_entry;
    // This member holds the program header table's file offset in
    // bytes. If the file has no program header table, this member
    // holds zero.
    Elf32_Off e_phoff;
    // This member holds the section header table's file offset in
    // bytes. If the file has no section header table, this member
    // holds zero.
    Elf32_Off e_shoff;
    // This member holds processor-specific flags associated with the
    // file. Flag names take the form EF_machine_flag.
    Elf32_Word e_flags;
    // This member holds the ELF header's size in bytes.
    Elf32_Half e_ehsize;
    // This member holds the size in bytes of one entry in the file's
    // program header table; all entries are the same size.
    Elf32_Half e_phentsize;
    // This member holds the number of entries in the program header
    // table. Thus the product of e_phentsize and e_phnum gives the
    // table's size in bytes. If a file has no program header table,
    // e_phnum holds the value zero.
    Elf32_Half e_phnum;
    // This member holds a section header's size in bytes. A section
    // header is one entry in the section header table; all entries
    // are the same size.
    Elf32_Half e_shentsize;
    // This member holds the number of entries in the section header
    // table.Thus the product of e_shentsize and e_shnum gives the
    // section header table's size in bytes. If a file has no section
    // header table, e_shnum holds the value zero.
    Elf32_Half e_shnum;
    // This member holds the section header table index of the entry
    // associated with the section name string table. If the file has
    // no section name string table, this member holds the value
    // SHN_UNDEF.
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    // This member specifies the name of the section. Its value is an
    // index into the section header string table section, giving the
    // location of a null-terminated string.
    Elf32_Word sh_name;
    // This member categorizes the section's contents and semantics.
    Elf32_Word sh_type;
    // Sections support 1-bit flags that describe miscellaneous
    // attributes.
    Elf32_Word sh_flags;
    // If the section will appear in the memory image of a process,
    // this member gives the address at which the section's first
    // byte should reside. Otherwise, the member contains 0.
    Elf32_Addr sh_addr;
    // This member's value gives the byte offset from the beginning
    // of the file to the first byte in the section. One section
    // type, SHT_NOBITS, occupies no space in the file, and its
    // sh_offset member locates the conceptual placement in the file.
    Elf32_Off  sh_offset;
    // This member gives the section's size in bytes. Unless the
    // section type is SHT_NOBITS, the section occupies sh_size bytes
    // in the file. A section of type SHT_NOBITS may have a non-zero
    // size, but it occupies no space in the file.
    Elf32_Word sh_size;
    // This member holds a section header table index link, whose
    // interpretation depends on the section type.
    Elf32_Word sh_link;
    // This member holds extra information, whose interpretation
    // depends on the section type.
    Elf32_Word sh_info;
    // Some sections have address alignment constraints. For example,
    // if a section holds a doubleword, the system must ensure
    // doubleword alignment for the entire section. That is, the
    // value of sh_addr must be congruent to 0 modulo the value of
    // sh_addralign. Currently, only 0 and positive integral powers
    // of two are allowed. Values 0 and 1 mean the section has no
    // alignment constraints.
    Elf32_Word sh_addralign;
    // Some sections hold a table of fixed-size entries, such as a
    // symbol table. For such a section, this member gives the size
    // in bytes of each entry. The member contains 0 if the section
    // does not hold a table of fixed-size entries.
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct {
    // This member holds an index into the object file’s symbol
    // string table, which holds the character representations of the
    // symbol names. If the value is non-zero, it represents a string
    // table index that gives the symbol name. Otherwise, the symbol
    // table entry has no name.
    Elf32_Word    st_name;
    // This member gives the value of the associated symbol.
    // Depending on the context, this may be an absolute value, an
    // address, etc.
    Elf32_Addr    st_value;
    // Many symbols have associated sizes. For example, a data
    // object’s size is the number of bytes contained in the object.
    // This member holds 0 if the symbol has no size or an unknown
    // size.
    Elf32_Word    st_size;
    // This member specifies the symbol’s type and binding
    // attributes. A list of the values and meanings appears below.
    // The following code shows how to manipulate the values.
    unsigned char st_info;
    // This member currently holds 0 and has no defined meaning.
    unsigned char st_other;
    // Every symbol table entry is "defined" in relation to some
    // section; this member holds the relevant section header table
    // index. Some section indices indicate special meanings.
    Elf32_Half    st_shndx;
} Elf32_Sym;

typedef struct {
    // This member gives the location at which to apply the
    // relocation action. For a relocatable file, the value is the
    // byte offset from the beginning of the section to the storage
    // unit affected by the relocation. For an executable file or a
    // shared object, the value is the virtual address of the storage
    // unit affected by the relocation.
    Elf32_Addr r_offset;
    // This member gives both the symbol table index with respect to
    // which the relocation must be made, and the type of relocation
    // to apply. For example, a call instruction’s relocation entry
    // would hold the symbol table index of the function being
    // called. If the index is STN_UNDEF, the undefined symbol index,
    // the relocation uses 0 as the ‘‘symbol value.’’ Relocation
    // types are processor-specific. When the text refers to a
    // relocation entry’s relocation type or symbol table index, it
    // means the result of applying ELF32_R_TYPE or ELF32_R_SYM,
    // respectively, to the entry’s r_info member
    Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
    // See Elf32_Rel
    Elf32_Addr  r_offset;
    // See Elf32_Rel
    Elf32_Word  r_info;
    // This member specifies a constant addend used to compute the
    // value to be stored into the relocatable field.
    Elf32_Sword r_addend;
} Elf32_Rela;

typedef struct {
    // This member tells what kind of segment this array element
    // describes or how to interpret the array element’s information.
    Elf32_Word p_type;
    // This member gives the offset from the beginning of the file at
    // which the first byte of the segment resides.
    Elf32_Off  p_offset;
    // This member gives the virtual address at which the first byte
    // of the segment resides in memory.
    Elf32_Addr p_vaddr;
    // On systems for which physical addressing is relevant, this
    // member is reserved for the segment’s physical address. Because
    // System V ignores physical addressing for application programs,
    // this member has unspecified contents for executable files and
    // shared objects.
    Elf32_Addr p_paddr;
    // This member gives the number of bytes in the file image of the
    // segment; it may be zero.
    Elf32_Word p_filesz;
    // This member gives the number of bytes in the memory image of
    // the segment; it may be zero.
    Elf32_Word p_memsz;
    // This member gives flags relevant to the segment.
    Elf32_Word p_flags;
    // Loadable process segments must have congruent values for
    // p_vaddr and p_offset, modulo the page size. This member gives
    // the value to which the segments are aligned in memory and in
    // the file. Values 0 and 1 mean no alignment is required.
    // Otherwise, p_align should be a positive, integral power of 2,
    // and p_vaddr should equal p_offset, modulo p_align.
    Elf32_Word p_align;
} Elf32_Phdr;

typedef struct {
    Elf32_Sword d_tag;
    union {
        // These Elf32_Word objects represent integer values with
        // various interpretations.
        Elf32_Word d_val;
        // These Elf32_Addr objects represent program virtual
        // addresses. As mentioned previously, a file’s virtual
        // addresses might not match the memory virtual addresses
        // during execution. When interpreting addresses contained in
        // the dynamic structure, the dynamic linker computes actual
        // addresses, based on the original file value and the memory
        // base address. For consistency, files do not contain
        // relocation entries to ‘‘correct’’ addresses in the dynamic
        // structure.
        Elf32_Addr d_ptr;
    } d_un;
} Elf32_Dyn;

inline unsigned long
elf_hash(const unsigned char* name)
{
    unsigned long h = 0, g;
    while (*name) {
        h = (h << 4) + *name++;
        if (g = h & 0xF0000000)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}


enum {
    // Type Class Operation  
    // ------------------------------
    R_ARM_NONE                  =  0,  // Static Misc
    R_ARM_PC24                  =  1,  // Deprecated ARM ((S + A) | T) - P
    R_ARM_ABS32                 =  2,  // Static Data (S + A) | T
    R_ARM_REL32                 =  3,  // Static Data ((S + A) | T) - P
    R_ARM_LDR_PC_G0             =  4,  // Static ARM S + A - P
    R_ARM_ABS16                 =  5,  // Static Data S + A
    R_ARM_ABS12                 =  6,  // Static ARM S + A
    R_ARM_THM_ABS5              =  7,  // Static Thumb16 S + A
    R_ARM_ABS8                  =  8,  // Static Data S + A
    R_ARM_SBREL32               =  9,  // Static Data ((S + A) | T) - B(S)
    R_ARM_THM_CALL              =  10, // Static Thumb32 ((S + A) | T) - P
    R_ARM_THM_PC8               =  11, // Static Thumb16
    R_ARM_BREL_ADJ              =  12, // Dynamic Data DeltaB(S) + A
    R_ARM_TLS_DESC              =  13, // Dynamic Data
    R_ARM_THM_SWI8              =  14, // Obsolete
    R_ARM_XPC25                 =  15, // Obsolete
    R_ARM_THM_XPC22             =  16, // Obsolete
    R_ARM_TLS_DTPMOD32          =  17, // Dynamic Data Module(S)
    R_ARM_TLS_DTPOFF32          =  18, // Dynamic Data S + A - TLS
    R_ARM_TLS_TPOFF32           =  19, // Dynamic Data S + A - tp
    R_ARM_COPY                  =  20, // Dynamic Misc
    R_ARM_GLOB_DAT              =  21, // Dynamic Data (S + A) | T
    R_ARM_JUMP_SLOT             =  22, // Dynamic Data (S + A) | T
    R_ARM_RELATIVE              =  23, // Dynamic Data B(S) + A
    R_ARM_GOTOFF32              =  24, // Static Data (((S + A) | T) - GOT_ORG
    R_ARM_BASE_PREL             =  25, // Static Data B(S) + A - P
    R_ARM_GOT_BREL              =  26, // Static Data GOT(S) + A - GOT_ORG
    R_ARM_PLT32                 =  27, // Deprecated ARM ((S + A) | T) - P
    R_ARM_CALL                  =  28, // Static ARM ((S + A) | T) - P
    R_ARM_JUMP24                =  29, // Static ARM ((S + A) | T) - P
    R_ARM_THM_JUMP24            =  30, // Static Thumb32 ((S + A) | T) - P
    R_ARM_BASE_ABS              =  31, // Static Data B(S) + A
    R_ARM_ALU_PCREL_7_0         =  32, // Obsolete
    R_ARM_ALU_PCREL_15_8        =  33, // Obsolete
    R_ARM_ALU_PCREL_23_15       =  34, // Obsolete
    R_ARM_LDR_SBREL_11_0_NC     =  35, // Deprecated ARM S + A - B(S)
    R_ARM_ALU_SBREL_19_12_NC    =  36, // Deprecated ARM S + A - B(S)
    R_ARM_ALU_SBREL_27_20_CK    =  37, // Deprecated ARM S + A - B(S)
    R_ARM_TARGET1               =  38, // Data  Misc (S + A) | T or //   ((S + A) | T) - P
    R_ARM_SBREL31               =  39, // Deprecated Data ((S + A) | T) - B(S)
    R_ARM_V4BX                  =  40, // Static Misc 
    R_ARM_TARGET2               =  41, // Static Misc
    R_ARM_PREL31                =  42, // Static Data ((S + A) | T) - P
    R_ARM_MOVW_ABS_NC           =  43, // Static ARM (S + A) | T
    R_ARM_MOVT_ABS              =  44, // Static ARM S + A
    R_ARM_MOVW_PREL_NC          =  45, // Static ARM ((S + A) | T) - P
    R_ARM_MOVT_PREL             =  46, // Static ARM S + A - P
    R_ARM_THM_MOVW_ABS_NC       =  47, // Static Thumb32 (S + A) | T
    R_ARM_THM_MOVT_ABS          =  48, // Static Thumb32 S + A - P
    R_ARM_THM_MOVW_PREL_NC      =  49, // Static Thumb32 ((S + A) | T) - P
    R_ARM_THM_MOVT_PREL         =  50, // Static Thumb32 S + A - P
    R_ARM_THM_JUMP19            =  51, // Static Thumb32 ((S + A) | T) - P
    R_ARM_THM_JUMP6             =  52, // Static Thumb16 S + A - P
    R_ARM_THM_ALU_PREL_11_0     =  53, // Static Thumb32 ((S + A) | T) - Pa
    R_ARM_THM_PC12              =  54, // Static Thumb32 S + A - Pa
    R_ARM_ABS32_NOI             =  55, // Static Data S + A
    R_ARM_REL32_NOI             =  56, // Static Data S + A - P
    R_ARM_ALU_PC_G0_NC          =  57, // Static ARM ((S + A) | T) - P
    R_ARM_ALU_PC_G0             =  58, // Static ARM ((S + A) | T) - P
    R_ARM_ALU_PC_G1_NC          =  59, // Static ARM ((S + A) | T) - P
    R_ARM_ALU_PC_G1             =  60, // Static ARM ((S + A) | T) - P
    R_ARM_ALU_PC_G2             =  61, // Static ARM ((S + A) | T) - P
    R_ARM_LDR_PC_G1             =  62, // Static ARM S + A - P
    R_ARM_LDR_PC_G2             =  63, // Static ARM S + A - P
    R_ARM_LDRS_PC_G0            =  64, // Static ARM S + A - P
    R_ARM_LDRS_PC_G1            =  65, // Static ARM S + A - P
    R_ARM_LDRS_PC_G2            =  66, // Static ARM S + A - P
    R_ARM_LDC_PC_G0             =  67, // Static ARM S + A - P
    R_ARM_LDC_PC_G1             =  68, // Static ARM S + A - P
    R_ARM_LDC_PC_G2             =  69, // Static ARM S + A - P
    R_ARM_ALU_SB_G0_NC          =  70, // Static ARM ((S + A) | T) - B(S)
    R_ARM_ALU_SB_G0             =  71, // Static ARM ((S + A) | T) - B(S)
    R_ARM_ALU_SB_G1_NC          =  72, // Static ARM ((S + A) | T) - B(S)
    R_ARM_ALU_SB_G1             =  73, // Static ARM ((S + A) | T) - B(S)
    R_ARM_ALU_SB_G2             =  74, // Static ARM ((S + A) | T) - B(S)
    R_ARM_LDR_SB_G0             =  75, // Static ARM S + A - B(S)
    R_ARM_LDR_SB_G1             =  76, // Static ARM S + A - B(S)
    R_ARM_LDR_SB_G2             =  77, // Static ARM S + A - B(S)
    R_ARM_LDRS_SB_G0            =  78, // Static ARM S + A - B(S)
    R_ARM_LDRS_SB_G1            =  79, // Static ARM S + A - B(S)
    R_ARM_LDRS_SB_G2            =  80, // Static ARM S + A - B(S)
    R_ARM_LDC_SB_G0             =  81, // Static ARM S + A - B(S)
    R_ARM_LDC_SB_G1             =  82, // Static ARM S + A - B(S)
    R_ARM_LDC_SB_G2             =  83, // Static ARM S + A - B(S)
    R_ARM_MOVW_BREL_NC          =  84, // Static ARM ((S + A) | T) - B(S)
    R_ARM_MOVT_BREL             =  85, // Static ARM S + A - B(S)
    R_ARM_MOVW_BREL             =  86, // Static ARM ((S + A) | T) - B(S)
    R_ARM_THM_MOVW_BREL_NC      =  87, // Static Thumb32 ((S + A) | T) - B(S)
    R_ARM_THM_MOVT_BREL         =  88, // Static Thumb32 S + A - B(S)
    R_ARM_THM_MOVW_BREL         =  89, // Static Thumb32 ((S + A) | T) - B(S)
    R_ARM_TLS_GOTDESC           =  90, // Static Data
    R_ARM_TLS_CALL              =  91, // Static ARM
    R_ARM_TLS_DESCSEQ           =  92, // Static ARM TLS relaxation
    R_ARM_THM_TLS_CALL          =  93, // Static Thumb32
    R_ARM_PLT32_ABS             =  94, // Static Data PLT(S) + A
    R_ARM_GOT_ABS               =  95, // Static Data GOT(S) + A
    R_ARM_GOT_PREL              =  96, // Static Data GOT(S) + A - P
    R_ARM_GOT_BREL12            =  97, // Static ARM GOT(S) + A - GOT_ORG
    R_ARM_GOTOFF12              =  98, // Static ARM S + A - GOT_ROG
    R_ARM_GOTRELAX              =  99, // Static Misc
    R_ARM_GNU_VTENTRY           = 100, // Deprecated Data
    R_ARM_GNU_VTINHERIT         = 101, // Deprecated Data
    R_ARM_THM_JUMP11            = 102, // Static Thumb16 S + A - P
    R_ARM_THM_JUMP8             = 103, // Static Thumb16 S + A - P
    R_ARM_TLS_GD32              = 104, // Static Data GOT(S) + A - P
    R_ARM_TLS_LDM32             = 105, // Static Data GOT(S) + A - P
    R_ARM_TLS_LDO32             = 106, // Static Data S + A - TLS
    R_ARM_TLS_IE32              = 107, // Static Data GOT(S) + A - P
    R_ARM_TLS_LE32              = 108, // Static Data S + A - tp
    R_ARM_TLS_LDO12             = 109, // Static ARM S + A - TLS
    R_ARM_TLS_LE12              = 110, // Static ARM S + A - tp
    R_ARM_TLS_IE12GP            = 111, // Static ARM GOT(S) + A - GOT_ORG
    R_ARM_PRIVATE_0             = 112, // Private (n = 0, 1, ... 15)
    R_ARM_PRIVATE_1             = 113,
    R_ARM_PRIVATE_2             = 114,
    R_ARM_PRIVATE_3             = 115,
    R_ARM_PRIVATE_4             = 116,
    R_ARM_PRIVATE_5             = 117,
    R_ARM_PRIVATE_6             = 118,
    R_ARM_PRIVATE_7             = 119,
    R_ARM_PRIVATE_8             = 120,
    R_ARM_PRIVATE_9             = 121,
    R_ARM_PRIVATE_10            = 122,
    R_ARM_PRIVATE_11            = 123,
    R_ARM_PRIVATE_12            = 124,
    R_ARM_PRIVATE_13            = 125,
    R_ARM_PRIVATE_14            = 126,
    R_ARM_PRIVATE_15            = 127,
    R_ARM_ME_TOO                = 128, // Obsolete
    R_ARM_THM_TLS_DESCSEQ16     = 129, // Static Thumb16
    R_ARM_THM_TLS_DESCSEQ32     = 130, // Static Thumb32
    // 131 - 135   Unallocated
    // Relocations for Armv8.1-M Mainline (BF/BFL)
    R_ARM_THM_BF16              = 136, // Static       Thumb32 ((S + A) | T) – P
    R_ARM_THM_BF12              = 137, // Static       Thumb32 ((S + A) | T) – P
    R_ARM_THM_BF18              = 138, // Static       Thumb32 ((S + A) | T) – P
    // 139   Unallocated
    // 140 - 159   Dynamic  Reserved for future allocation
    R_ARM_IRELATIVE             = 160, // Dynamic
    // 161 - 255   Unallocated
};



/**
 * Special Sections
 * 
 * Name           Type            Attributes
 * --------------------------------------------------------
 * .bss         | SHT_NOBITS    | SHF_ALLOC + SHF_WRITE
 * .comment     | SHT_PROGBITS  | none
 * .data        | SHT_PROGBITS  | SHF_ALLOC + SHF_WRITE
 * .data1       | SHT_PROGBITS  | SHF_ALLOC + SHF_WRITE
 * .debug       | SHT_PROGBITS  | none
 * .dynamic     | SHT_DYNAMIC   | --
 * .dynstr      | SHT_STRTAB    | SHF_ALLOC
 * .dynsym      | SHT_DYNSYM    | SHF_ALLOC
 * .fini        | SHT_PROGBITS  | SHF_ALLOC + SHF_EXECINSTR
 * .got         | SHT_PROGBITS  | --
 * .hash        | SHT_HASH      | SHF_ALLOC
 * .init        | SHT_PROGBITS  | SHF_ALLOC + SHF_EXECINSTR
 * .interp      | SHT_PROGBITS  | --
 * .line        | SHT_PROGBITS  | none
 * .note        | SHT_NOTE      | none
 * .plt         | SHT_PROGBITS  | --
 * .rel<name>   | SHT_REL       | --
 * .rela<name>  | SHT_RELA      | --
 * .rodata      | SHT_PROGBITS  | SHF_ALLOC
 * .rodata1     | SHT_PROGBITS  | SHF_ALLOC
 * .shstrtab    | SHT_STRTAB    | none
 * .strtab      | SHT_STRTAB    | --
 * .symtab      | SHT_SYMTAB    | --
 * .text        | SHT_PROGBITS  | SHF_ALLOC + SHF_EXECINSTR
 * --------------------------------------------------------
 * 
 * .bss        This section holds uninitialized data that contribute
 *             to the program’s memory image. By definition, the
 *             system initializes the data with zeros when the
 *             program begins to run. The section occupies no file
 *             space, as indicated by the section type, SHT_NOBITS.
 * .comment    This section holds version control information.
 * .data
 * .data1      These sections hold initialized data that contribute
 *             to the program’s memory image.
 * .debug      This section holds information for symbolic debugging.
 *             The contents are unspecified.
 * .dynamic    This section holds dynamic linking information. The
 *             section’s attributes will include the SHF_ALLOC bit.
 *             Whether the SHF_WRITE bit is set is processor
 *             specific.
 * .dynstr     This section holds strings needed for dynamic linking,
 *             most commonly the strings that represent the names
 *             associated with symbol table entries.
 * .dynsym     This section holds the dynamic linking symbol table.
 * .fini       This section holds executable instructions that
 *             contribute to the process termination code. That is,
 *             when a program exits normally, the system arranges to
 *             execute the code in this section.
 * .got        This section holds the global offset table.
 * .hash       This section holds a symbol hash table.
 * .init       This section holds executable instructions that
 *             contribute to the process initialization code. That
 *             is, when a program starts to run, the system arranges
 *             to execute the code in this section before calling the
 *             main program entry point (called main for C programs).
 * .interp     This section holds the path name of a program
 *             interpreter. If the file has a loadable segment that
 *             includes the section, the section’s attributes will
 *             include the SHF_ALLOC bit; otherwise, that bit will be
 *             off.
 * .line       This section holds line number information for
 *             symbolic debugging, which describes the correspondence
 *             between the source program and the machine code. The
 *             contents are unspecified.
 * .note       This section holds information in a "Note Section"
 *             format.
 * .plt        This section holds the procedure linkage table.
 * .rel<name>
 * .rela<name> These sections hold relocation information. If the
 *             file has a loadable segment that includes relocation,
 *             the sections’ attributes will include the SHF_ALLOC
 *             bit; otherwise, that bit will be off. Conventionally,
 *             name is supplied by the section to which the
 *             relocations apply. Thus a relocation section for .text
 *             normally would have the name .rel.text or .rela.text.
 * .rodata
 * .rodata1    These sections hold read-only data that typically
 *             contribute to a non-writable segment in the process
 *             image.
 * .shstrtab   This section holds section names.
 * .strtab     This section holds strings, most commonly the strings
 *             that represent the names associated with symbol table
 *             entries. If the file has a loadable segment that
 *             includes the symbol string table, the section’s
 *             attributes will include the SHF_ALLOC bit; otherwise,
 *             that bit will be off.
 * .symtab     This section holds a symbol table. If the file has a
 *             loadable segment that includes the symbol table, the
 *             section’s attributes will include the SHF_ALLOC bit;
 *             otherwise, that bit will be off.
 * .text       This section holds the "text," or executable
 *             instructions, of a program.
 */

#endif
