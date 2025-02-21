#
# Unusual variables checked by this code:
#	NOP - two byte opcode for no-op (defaults to 0)
#	DATA_ADDR - if end-of-text-plus-one-page isn't right for data start
#	OTHER_READONLY_SECTIONS - other than .text .init .rodata ...
#		(e.g., .PARISC.milli)
#	OTHER_READWRITE_SECTIONS - other than .data .bss .ctors .sdata ...
#		(e.g., .PARISC.global)
#	OTHER_SECTIONS - at the end
#	EXECUTABLE_SYMBOLS - symbols that must be defined for an
#		executable (e.g., _DYNAMIC_LINK)
#	TEXT_START_SYMBOLS - symbols that appear at the start of the
#		.text section.
#	DATA_START_SYMBOLS - symbols that appear at the start of the
#		.data section.
#	OTHER_BSS_SYMBOLS - symbols that appear at the start of the
#		.bss section besides __bss_start.
#	EMBEDDED - whether this is for an embedded system. 
#
# When adding sections, do note that the names of some sections are used
# when specifying the start address of the next.
#

# We use a start address of __start for Irix 5, _start for other
# targets.  This is for compatibility with Irix 5, and with old MIPS
# ELF toolchains.
if [ -z "$ENTRY" ]; then
  case "${target}" in
  mips*-*-irix5*) ENTRY=__start ;;
  *) ENTRY=_start ;;
  esac
fi

# if this is for an embedded system, don't add SIZEOF_HEADERS.
if [ -z "$EMBEDDED" ]; then
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR} + SIZEOF_HEADERS"
else
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR}"
fi

test -z "${BIG_OUTPUT_FORMAT}" && BIG_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test -z "${LITTLE_OUTPUT_FORMAT}" && LITTLE_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test "$LD_FLAG" = "N" && DATA_ADDR=.
INTERP=".interp   ${RELOCATING-0} : { *(.interp) 	}"
cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}",
	      "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
ENTRY(${ENTRY})

${RELOCATING+${LIB_SEARCH_DIRS}}
${RELOCATING+/* Do we need any of these for elf?
   __DYNAMIC = 0; ${STACKZERO+${STACKZERO}} ${SHLIB_PATH+${SHLIB_PATH}}  */}
${RELOCATING+${EXECUTABLE_SYMBOLS}}
${RELOCATING- /* For some reason, the Solaris linker makes bad executables
  if gld -r is used and the intermediate file has sections starting
  at non-zero addresses.  Could be a Solaris ld bug, could be a GNU ld
  bug.  But for now assigning the zero vmas works.  */}
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${CREATE_SHLIB-${RELOCATING+. = ${TEXT_BASE_ADDRESS};}}
  ${CREATE_SHLIB+${RELOCATING+. = ${SHLIB_TEXT_START_ADDR} + SIZEOF_HEADERS;}}
  ${CREATE_SHLIB-${INTERP}}
  .reginfo     ${RELOCATING-0} : { *(.reginfo) }
  .dynamic     ${RELOCATING-0} : { *(.dynamic) }
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .hash        ${RELOCATING-0} : { *(.hash)		}
  .rel.text    ${RELOCATING-0} : { *(.rel.text)		}
  .rela.text   ${RELOCATING-0} : { *(.rela.text) 	}
  .rel.data    ${RELOCATING-0} : { *(.rel.data)		}
  .rela.data   ${RELOCATING-0} : { *(.rela.data) 	}
  .rel.rodata  ${RELOCATING-0} : { *(.rel.rodata) 	}
  .rela.rodata ${RELOCATING-0} : { *(.rela.rodata) 	}
  .rel.got     ${RELOCATING-0} : { *(.rel.got)		}
  .rela.got    ${RELOCATING-0} : { *(.rela.got)		}
  .rel.ctors   ${RELOCATING-0} : { *(.rel.ctors)	}
  .rela.ctors  ${RELOCATING-0} : { *(.rela.ctors)	}
  .rel.dtors   ${RELOCATING-0} : { *(.rel.dtors)	}
  .rela.dtors  ${RELOCATING-0} : { *(.rela.dtors)	}
  .rel.init    ${RELOCATING-0} : { *(.rel.init)	}
  .rela.init   ${RELOCATING-0} : { *(.rela.init)	}
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini)	}
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini)	}
  .rel.bss     ${RELOCATING-0} : { *(.rel.bss)		}
  .rela.bss    ${RELOCATING-0} : { *(.rela.bss)		}
  .rel.plt     ${RELOCATING-0} : { *(.rel.plt)		}
  .rela.plt    ${RELOCATING-0} : { *(.rela.plt)		}
  .rodata  ${RELOCATING-0} : { *(.rodata)  }
  .rodata1 ${RELOCATING-0} : { *(.rodata1) }
  .init        ${RELOCATING-0} : { *(.init)	} =${NOP-0}
  .text    ${RELOCATING-0} :
  {
    ${CREATE_SHLIB-${RELOCATING+${TEXT_START_SYMBOLS}}}
    *(.text)
    *(.stub)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } =${NOP-0}
  ${CREATE_SHLIB-${RELOCATING+_etext = .;}}
  ${CREATE_SHLIB-${RELOCATING+PROVIDE (etext = .);}}
  .fini    ${RELOCATING-0} : { *(.fini)    } =${NOP-0}

  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  It would
     be more correct to do this:
       ${RELOCATING+. = ${DATA_ADDR-ALIGN(${MAXPAGESIZE})
		+ ((ALIGN(8) + ${MAXPAGESIZE} - ALIGN(${MAXPAGESIZE}))
		   & (${MAXPAGESIZE} - 1)};}
     The current expression does not correctly handle the case of a
     text segment ending precisely at the end of a page; it causes the
     data segment to skip a page.  The above expression does not have
     this problem, but it will currently (2/95) cause BFD to allocate
     a single segment, combining both text and data, for this case.
     This will prevent the text segment from being shared among
     multiple executions of the program; I think that is more
     important than losing a page of the virtual address space (note
     that no actual memory is lost; the page which is skipped can not
     be referenced).  */
  ${CREATE_SHLIB-${RELOCATING+. += ${DATA_ADDR} - ${TEXT_START_ADDR};}}
  ${CREATE_SHLIB+${RELOCATING+. += 0x10000;}}
  .data  ${RELOCATING-0} :
  {
    ${CREATE_SHLIB-${RELOCATING+${DATA_START_SYMBOLS}}}
    *(.data)
    ${CONSTRUCTING+CONSTRUCTORS}
  }
  .data1 ${RELOCATING-0} : { *(.data1) }
  .ctors       ${RELOCATING-0} : { *(.ctors)   }
  .dtors       ${RELOCATING-0} : { *(.dtors)   }
  ${RELOCATING+${OTHER_GOT_SYMBOLS}}
  .got         ${RELOCATING-0} :
  {
    *(.got.plt) *(.got)
   }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata   ${RELOCATING-0} : { *(.sdata) }
  ${RELOCATING+${OTHER_READWRITE_SECTIONS}}
  ${CREATE_SHLIB-${RELOCATING+_edata  =  .;}}
  ${CREATE_SHLIB-${RELOCATING+PROVIDE (edata = .);}}
  ${CREATE_SHLIB-${RELOCATING+__bss_start = .;}}
  ${CREATE_SHLIB-${RELOCATING+${OTHER_BSS_SYMBOLS}}}
  .sbss    ${RELOCATING-0} : { *(.sbss) *(.scommon) }
  .bss     ${RELOCATING-0} :
  {
   *(.dynbss)
   *(.bss)
   *(COMMON)
  }
  ${CREATE_SHLIB-${RELOCATING+_end = . ;}}
  ${CREATE_SHLIB-${RELOCATING+PROVIDE (end = .);}}

  /* These are needed for ELF backends which have not yet been
     converted to the new style linker.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }

  /* DWARF debug sections.
     Symbols in the .debug DWARF section are relative to the beginning of the
     section so we begin .debug at 0.  It's not clear yet what needs to happen
     for the others.   */
  .debug          0 : { *(.debug) }
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  .line           0 : { *(.line) }

  /* These must appear regardless of ${RELOCATING}.  */
  ${OTHER_SECTIONS}
}
EOF
