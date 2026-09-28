OUTPUT_ARCH("riscv")
ENTRY(_start)
PHDRS
{
  text_phdr PT_LOAD FLAGS(5); /* 5 = Read + Execute (RX) */
  data_phdr PT_LOAD FLAGS(6); /* 6 = Read + Write (RW) */
}

SECTIONS
{
  . = 0x80000000;
  .text : { *(.text*) } :text_phdr
  .data : { *(.data*) } :data_phdr
  .bss  : { *(.bss*)  } :data_phdr
}
