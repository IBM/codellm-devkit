#objdump: -dr
#name: MIPS ulh-xgot
#as: -mips1 -KPIC -xgot
#source: ulh-pic.s

# Test the unaligned load and store macros with -KPIC -xgot.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+0000 R_MIPS_GOT16 .data
...
0+0008 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+0008 R_MIPS_LO16 .data
...
0+0010 <[^>]*> lb \$a0,0\(\$at\)
0+0014 <[^>]*> lbu \$at,1\(\$at\)
0+0018 <[^>]*> sll \$a0,\$a0,0x8
0+001c <[^>]*> or \$a0,\$a0,\$at
0+0020 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0020 R_MIPS_GOT_HI16 big_external_data_label
0+0024 <[^>]*> addu \$at,\$at,\$gp
0+0028 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0028 R_MIPS_GOT_LO16 big_external_data_label
...
0+0030 <[^>]*> lbu \$a0,0\(\$at\)
0+0034 <[^>]*> lbu \$at,1\(\$at\)
0+0038 <[^>]*> sll \$a0,\$a0,0x8
0+003c <[^>]*> or \$a0,\$a0,\$at
0+0040 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0040 R_MIPS_GOT_HI16 small_external_data_label
0+0044 <[^>]*> addu \$at,\$at,\$gp
0+0048 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0048 R_MIPS_GOT_LO16 small_external_data_label
...
0+0050 <[^>]*> lwl \$a0,0\(\$at\)
0+0054 <[^>]*> lwr \$a0,3\(\$at\)
0+0058 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0058 R_MIPS_GOT_HI16 big_external_common
0+005c <[^>]*> addu \$at,\$at,\$gp
0+0060 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0060 R_MIPS_GOT_LO16 big_external_common
...
0+0068 <[^>]*> sb \$a0,1\(\$at\)
0+006c <[^>]*> srl \$a0,\$a0,0x8
0+0070 <[^>]*> sb \$a0,0\(\$at\)
0+0074 <[^>]*> lbu \$at,1\(\$at\)
0+0078 <[^>]*> sll \$a0,\$a0,0x8
0+007c <[^>]*> or \$a0,\$a0,\$at
0+0080 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0080 R_MIPS_GOT_HI16 small_external_common
0+0084 <[^>]*> addu \$at,\$at,\$gp
0+0088 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0088 R_MIPS_GOT_LO16 small_external_common
...
0+0090 <[^>]*> swl \$a0,0\(\$at\)
0+0094 <[^>]*> swr \$a0,3\(\$at\)
0+0098 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+0098 R_MIPS_GOT16 .bss
...
0+00a0 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+00a0 R_MIPS_LO16 .bss
...
0+00a8 <[^>]*> lb \$a0,0\(\$at\)
0+00ac <[^>]*> lbu \$at,1\(\$at\)
0+00b0 <[^>]*> sll \$a0,\$a0,0x8
0+00b4 <[^>]*> or \$a0,\$a0,\$at
0+00b8 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+00b8 R_MIPS_GOT16 .bss
...
0+00c0 <[^>]*> addiu \$at,\$at,1000
[ 	]*RELOC: 0+00c0 R_MIPS_LO16 .bss
...
0+00c8 <[^>]*> lbu \$a0,0\(\$at\)
0+00cc <[^>]*> lbu \$at,1\(\$at\)
0+00d0 <[^>]*> sll \$a0,\$a0,0x8
0+00d4 <[^>]*> or \$a0,\$a0,\$at
0+00d8 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+00d8 R_MIPS_GOT16 .data
...
0+00e0 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+00e0 R_MIPS_LO16 .data
...
0+00e8 <[^>]*> addiu \$at,\$at,1
0+00ec <[^>]*> lwl \$a0,0\(\$at\)
0+00f0 <[^>]*> lwr \$a0,3\(\$at\)
0+00f4 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00f4 R_MIPS_GOT_HI16 big_external_data_label
0+00f8 <[^>]*> addu \$at,\$at,\$gp
0+00fc <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+00fc R_MIPS_GOT_LO16 big_external_data_label
...
0+0104 <[^>]*> addiu \$at,\$at,1
0+0108 <[^>]*> sb \$a0,1\(\$at\)
0+010c <[^>]*> srl \$a0,\$a0,0x8
0+0110 <[^>]*> sb \$a0,0\(\$at\)
0+0114 <[^>]*> lbu \$at,1\(\$at\)
0+0118 <[^>]*> sll \$a0,\$a0,0x8
0+011c <[^>]*> or \$a0,\$a0,\$at
0+0120 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0120 R_MIPS_GOT_HI16 small_external_data_label
0+0124 <[^>]*> addu \$at,\$at,\$gp
0+0128 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0128 R_MIPS_GOT_LO16 small_external_data_label
...
0+0130 <[^>]*> addiu \$at,\$at,1
0+0134 <[^>]*> swl \$a0,0\(\$at\)
0+0138 <[^>]*> swr \$a0,3\(\$at\)
0+013c <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+013c R_MIPS_GOT_HI16 big_external_common
0+0140 <[^>]*> addu \$at,\$at,\$gp
0+0144 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0144 R_MIPS_GOT_LO16 big_external_common
...
0+014c <[^>]*> addiu \$at,\$at,1
0+0150 <[^>]*> lb \$a0,0\(\$at\)
0+0154 <[^>]*> lbu \$at,1\(\$at\)
0+0158 <[^>]*> sll \$a0,\$a0,0x8
0+015c <[^>]*> or \$a0,\$a0,\$at
0+0160 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0160 R_MIPS_GOT_HI16 small_external_common
0+0164 <[^>]*> addu \$at,\$at,\$gp
0+0168 <[^>]*> lw \$at,0\(\$at\)
[ 	]*RELOC: 0+0168 R_MIPS_GOT_LO16 small_external_common
...
0+0170 <[^>]*> addiu \$at,\$at,1
0+0174 <[^>]*> lbu \$a0,0\(\$at\)
0+0178 <[^>]*> lbu \$at,1\(\$at\)
0+017c <[^>]*> sll \$a0,\$a0,0x8
0+0180 <[^>]*> or \$a0,\$a0,\$at
0+0184 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+0184 R_MIPS_GOT16 .bss
...
0+018c <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+018c R_MIPS_LO16 .bss
...
0+0194 <[^>]*> addiu \$at,\$at,1
0+0198 <[^>]*> lwl \$a0,0\(\$at\)
0+019c <[^>]*> lwr \$a0,3\(\$at\)
0+01a0 <[^>]*> lw \$at,0\(\$gp\)
[ 	]*RELOC: 0+01a0 R_MIPS_GOT16 .bss
...
0+01a8 <[^>]*> addiu \$at,\$at,1000
[ 	]*RELOC: 0+01a8 R_MIPS_LO16 .bss
...
0+01b0 <[^>]*> addiu \$at,\$at,1
0+01b4 <[^>]*> sb \$a0,1\(\$at\)
0+01b8 <[^>]*> srl \$a0,\$a0,0x8
0+01bc <[^>]*> sb \$a0,0\(\$at\)
0+01c0 <[^>]*> lbu \$at,1\(\$at\)
0+01c4 <[^>]*> sll \$a0,\$a0,0x8
0+01c8 <[^>]*> or \$a0,\$a0,\$at
...
