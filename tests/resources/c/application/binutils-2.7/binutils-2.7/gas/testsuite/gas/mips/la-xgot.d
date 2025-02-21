#objdump: -dr
#name: MIPS la-xgot
#as: -mips1 -KPIC -xgot
#source: la.s

# Test the la macro with -KPIC -xgot.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> li \$a0,0
0+0004 <[^>]*> li \$a0,1
0+0008 <[^>]*> li \$a0,0x8000
0+000c <[^>]*> li \$a0,-32768
0+0010 <[^>]*> lui \$a0,0x1
0+0014 <[^>]*> lui \$a0,0x1
0+0018 <[^>]*> ori \$a0,\$a0,0xa5a5
0+001c <[^>]*> li \$a0,0
0+0020 <[^>]*> addu \$a0,\$a0,\$a1
0+0024 <[^>]*> li \$a0,1
0+0028 <[^>]*> addu \$a0,\$a0,\$a1
0+002c <[^>]*> li \$a0,0x8000
0+0030 <[^>]*> addu \$a0,\$a0,\$a1
0+0034 <[^>]*> li \$a0,-32768
0+0038 <[^>]*> addu \$a0,\$a0,\$a1
0+003c <[^>]*> lui \$a0,0x1
0+0040 <[^>]*> addu \$a0,\$a0,\$a1
0+0044 <[^>]*> lui \$a0,0x1
0+0048 <[^>]*> ori \$a0,\$a0,0xa5a5
0+004c <[^>]*> addu \$a0,\$a0,\$a1
0+0050 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0050 R_MIPS_GOT16 .data
...
0+0058 <[^>]*> addiu \$a0,\$a0,0
[ 	]*RELOC: 0+0058 R_MIPS_LO16 .data
0+005c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+005c R_MIPS_GOT_HI16 big_external_data_label
0+0060 <[^>]*> addu \$a0,\$a0,\$gp
0+0064 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0064 R_MIPS_GOT_LO16 big_external_data_label
0+0068 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0068 R_MIPS_GOT_HI16 small_external_data_label
0+006c <[^>]*> addu \$a0,\$a0,\$gp
0+0070 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0070 R_MIPS_GOT_LO16 small_external_data_label
0+0074 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0074 R_MIPS_GOT_HI16 big_external_common
0+0078 <[^>]*> addu \$a0,\$a0,\$gp
0+007c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+007c R_MIPS_GOT_LO16 big_external_common
0+0080 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0080 R_MIPS_GOT_HI16 small_external_common
0+0084 <[^>]*> addu \$a0,\$a0,\$gp
0+0088 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0088 R_MIPS_GOT_LO16 small_external_common
0+008c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+008c R_MIPS_GOT16 .bss
...
0+0094 <[^>]*> addiu \$a0,\$a0,0
[ 	]*RELOC: 0+0094 R_MIPS_LO16 .bss
0+0098 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0098 R_MIPS_GOT16 .bss
...
0+00a0 <[^>]*> addiu \$a0,\$a0,1000
[ 	]*RELOC: 0+00a0 R_MIPS_LO16 .bss
0+00a4 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+00a4 R_MIPS_GOT16 .data
...
0+00ac <[^>]*> addiu \$a0,\$a0,1
[ 	]*RELOC: 0+00ac R_MIPS_LO16 .data
0+00b0 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+00b0 R_MIPS_GOT_HI16 big_external_data_label
0+00b4 <[^>]*> addu \$a0,\$a0,\$gp
0+00b8 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+00b8 R_MIPS_GOT_LO16 big_external_data_label
...
0+00c0 <[^>]*> addiu \$a0,\$a0,1
0+00c4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+00c4 R_MIPS_GOT_HI16 small_external_data_label
0+00c8 <[^>]*> addu \$a0,\$a0,\$gp
0+00cc <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+00cc R_MIPS_GOT_LO16 small_external_data_label
...
0+00d4 <[^>]*> addiu \$a0,\$a0,1
0+00d8 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+00d8 R_MIPS_GOT_HI16 big_external_common
0+00dc <[^>]*> addu \$a0,\$a0,\$gp
0+00e0 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+00e0 R_MIPS_GOT_LO16 big_external_common
...
0+00e8 <[^>]*> addiu \$a0,\$a0,1
0+00ec <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+00ec R_MIPS_GOT_HI16 small_external_common
0+00f0 <[^>]*> addu \$a0,\$a0,\$gp
0+00f4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+00f4 R_MIPS_GOT_LO16 small_external_common
...
0+00fc <[^>]*> addiu \$a0,\$a0,1
0+0100 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0100 R_MIPS_GOT16 .bss
...
0+0108 <[^>]*> addiu \$a0,\$a0,1
[ 	]*RELOC: 0+0108 R_MIPS_LO16 .bss
0+010c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+010c R_MIPS_GOT16 .bss
...
0+0114 <[^>]*> addiu \$a0,\$a0,1001
[ 	]*RELOC: 0+0114 R_MIPS_LO16 .bss
0+0118 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0118 R_MIPS_GOT16 .data
0+011c <[^>]*> lui \$at,0x1
0+0120 <[^>]*> addiu \$at,\$at,-32768
[ 	]*RELOC: 0+0120 R_MIPS_LO16 .data
0+0124 <[^>]*> addu \$a0,\$a0,\$at
0+0128 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0128 R_MIPS_GOT_HI16 big_external_data_label
0+012c <[^>]*> addu \$a0,\$a0,\$gp
0+0130 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0130 R_MIPS_GOT_LO16 big_external_data_label
0+0134 <[^>]*> lui \$at,0x1
0+0138 <[^>]*> addiu \$at,\$at,-32768
0+013c <[^>]*> addu \$a0,\$a0,\$at
0+0140 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0140 R_MIPS_GOT_HI16 small_external_data_label
0+0144 <[^>]*> addu \$a0,\$a0,\$gp
0+0148 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0148 R_MIPS_GOT_LO16 small_external_data_label
0+014c <[^>]*> lui \$at,0x1
0+0150 <[^>]*> addiu \$at,\$at,-32768
0+0154 <[^>]*> addu \$a0,\$a0,\$at
0+0158 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0158 R_MIPS_GOT_HI16 big_external_common
0+015c <[^>]*> addu \$a0,\$a0,\$gp
0+0160 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0160 R_MIPS_GOT_LO16 big_external_common
0+0164 <[^>]*> lui \$at,0x1
0+0168 <[^>]*> addiu \$at,\$at,-32768
0+016c <[^>]*> addu \$a0,\$a0,\$at
0+0170 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0170 R_MIPS_GOT_HI16 small_external_common
0+0174 <[^>]*> addu \$a0,\$a0,\$gp
0+0178 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0178 R_MIPS_GOT_LO16 small_external_common
0+017c <[^>]*> lui \$at,0x1
0+0180 <[^>]*> addiu \$at,\$at,-32768
0+0184 <[^>]*> addu \$a0,\$a0,\$at
0+0188 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0188 R_MIPS_GOT16 .bss
0+018c <[^>]*> lui \$at,0x1
0+0190 <[^>]*> addiu \$at,\$at,-32768
[ 	]*RELOC: 0+0190 R_MIPS_LO16 .bss
0+0194 <[^>]*> addu \$a0,\$a0,\$at
0+0198 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0198 R_MIPS_GOT16 .bss
0+019c <[^>]*> lui \$at,0x1
0+01a0 <[^>]*> addiu \$at,\$at,-31768
[ 	]*RELOC: 0+01a0 R_MIPS_LO16 .bss
0+01a4 <[^>]*> addu \$a0,\$a0,\$at
0+01a8 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+01a8 R_MIPS_GOT16 .data
...
0+01b0 <[^>]*> addiu \$a0,\$a0,-32768
[ 	]*RELOC: 0+01b0 R_MIPS_LO16 .data
0+01b4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+01b4 R_MIPS_GOT_HI16 big_external_data_label
0+01b8 <[^>]*> addu \$a0,\$a0,\$gp
0+01bc <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+01bc R_MIPS_GOT_LO16 big_external_data_label
...
0+01c4 <[^>]*> addiu \$a0,\$a0,-32768
0+01c8 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+01c8 R_MIPS_GOT_HI16 small_external_data_label
0+01cc <[^>]*> addu \$a0,\$a0,\$gp
0+01d0 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+01d0 R_MIPS_GOT_LO16 small_external_data_label
...
0+01d8 <[^>]*> addiu \$a0,\$a0,-32768
0+01dc <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+01dc R_MIPS_GOT_HI16 big_external_common
0+01e0 <[^>]*> addu \$a0,\$a0,\$gp
0+01e4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+01e4 R_MIPS_GOT_LO16 big_external_common
...
0+01ec <[^>]*> addiu \$a0,\$a0,-32768
0+01f0 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+01f0 R_MIPS_GOT_HI16 small_external_common
0+01f4 <[^>]*> addu \$a0,\$a0,\$gp
0+01f8 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+01f8 R_MIPS_GOT_LO16 small_external_common
...
0+0200 <[^>]*> addiu \$a0,\$a0,-32768
0+0204 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0204 R_MIPS_GOT16 .bss
...
0+020c <[^>]*> addiu \$a0,\$a0,-32768
[ 	]*RELOC: 0+020c R_MIPS_LO16 .bss
0+0210 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0210 R_MIPS_GOT16 .bss
...
0+0218 <[^>]*> addiu \$a0,\$a0,-31768
[ 	]*RELOC: 0+0218 R_MIPS_LO16 .bss
0+021c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+021c R_MIPS_GOT16 .data
0+0220 <[^>]*> lui \$at,0x1
0+0224 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+0224 R_MIPS_LO16 .data
0+0228 <[^>]*> addu \$a0,\$a0,\$at
0+022c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+022c R_MIPS_GOT_HI16 big_external_data_label
0+0230 <[^>]*> addu \$a0,\$a0,\$gp
0+0234 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0234 R_MIPS_GOT_LO16 big_external_data_label
0+0238 <[^>]*> lui \$at,0x1
0+023c <[^>]*> addiu \$at,\$at,0
0+0240 <[^>]*> addu \$a0,\$a0,\$at
0+0244 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0244 R_MIPS_GOT_HI16 small_external_data_label
0+0248 <[^>]*> addu \$a0,\$a0,\$gp
0+024c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+024c R_MIPS_GOT_LO16 small_external_data_label
0+0250 <[^>]*> lui \$at,0x1
0+0254 <[^>]*> addiu \$at,\$at,0
0+0258 <[^>]*> addu \$a0,\$a0,\$at
0+025c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+025c R_MIPS_GOT_HI16 big_external_common
0+0260 <[^>]*> addu \$a0,\$a0,\$gp
0+0264 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0264 R_MIPS_GOT_LO16 big_external_common
0+0268 <[^>]*> lui \$at,0x1
0+026c <[^>]*> addiu \$at,\$at,0
0+0270 <[^>]*> addu \$a0,\$a0,\$at
0+0274 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0274 R_MIPS_GOT_HI16 small_external_common
0+0278 <[^>]*> addu \$a0,\$a0,\$gp
0+027c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+027c R_MIPS_GOT_LO16 small_external_common
0+0280 <[^>]*> lui \$at,0x1
0+0284 <[^>]*> addiu \$at,\$at,0
0+0288 <[^>]*> addu \$a0,\$a0,\$at
0+028c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+028c R_MIPS_GOT16 .bss
0+0290 <[^>]*> lui \$at,0x1
0+0294 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+0294 R_MIPS_LO16 .bss
0+0298 <[^>]*> addu \$a0,\$a0,\$at
0+029c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+029c R_MIPS_GOT16 .bss
0+02a0 <[^>]*> lui \$at,0x1
0+02a4 <[^>]*> addiu \$at,\$at,1000
[ 	]*RELOC: 0+02a4 R_MIPS_LO16 .bss
0+02a8 <[^>]*> addu \$a0,\$a0,\$at
0+02ac <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+02ac R_MIPS_GOT16 .data
0+02b0 <[^>]*> lui \$at,0x2
0+02b4 <[^>]*> addiu \$at,\$at,-23131
[ 	]*RELOC: 0+02b4 R_MIPS_LO16 .data
0+02b8 <[^>]*> addu \$a0,\$a0,\$at
0+02bc <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+02bc R_MIPS_GOT_HI16 big_external_data_label
0+02c0 <[^>]*> addu \$a0,\$a0,\$gp
0+02c4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+02c4 R_MIPS_GOT_LO16 big_external_data_label
0+02c8 <[^>]*> lui \$at,0x2
0+02cc <[^>]*> addiu \$at,\$at,-23131
0+02d0 <[^>]*> addu \$a0,\$a0,\$at
0+02d4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+02d4 R_MIPS_GOT_HI16 small_external_data_label
0+02d8 <[^>]*> addu \$a0,\$a0,\$gp
0+02dc <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+02dc R_MIPS_GOT_LO16 small_external_data_label
0+02e0 <[^>]*> lui \$at,0x2
0+02e4 <[^>]*> addiu \$at,\$at,-23131
0+02e8 <[^>]*> addu \$a0,\$a0,\$at
0+02ec <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+02ec R_MIPS_GOT_HI16 big_external_common
0+02f0 <[^>]*> addu \$a0,\$a0,\$gp
0+02f4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+02f4 R_MIPS_GOT_LO16 big_external_common
0+02f8 <[^>]*> lui \$at,0x2
0+02fc <[^>]*> addiu \$at,\$at,-23131
0+0300 <[^>]*> addu \$a0,\$a0,\$at
0+0304 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0304 R_MIPS_GOT_HI16 small_external_common
0+0308 <[^>]*> addu \$a0,\$a0,\$gp
0+030c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+030c R_MIPS_GOT_LO16 small_external_common
0+0310 <[^>]*> lui \$at,0x2
0+0314 <[^>]*> addiu \$at,\$at,-23131
0+0318 <[^>]*> addu \$a0,\$a0,\$at
0+031c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+031c R_MIPS_GOT16 .bss
0+0320 <[^>]*> lui \$at,0x2
0+0324 <[^>]*> addiu \$at,\$at,-23131
[ 	]*RELOC: 0+0324 R_MIPS_LO16 .bss
0+0328 <[^>]*> addu \$a0,\$a0,\$at
0+032c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+032c R_MIPS_GOT16 .bss
0+0330 <[^>]*> lui \$at,0x2
0+0334 <[^>]*> addiu \$at,\$at,-22131
[ 	]*RELOC: 0+0334 R_MIPS_LO16 .bss
0+0338 <[^>]*> addu \$a0,\$a0,\$at
0+033c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+033c R_MIPS_GOT16 .data
...
0+0344 <[^>]*> addiu \$a0,\$a0,0
[ 	]*RELOC: 0+0344 R_MIPS_LO16 .data
0+0348 <[^>]*> addu \$a0,\$a0,\$a1
0+034c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+034c R_MIPS_GOT_HI16 big_external_data_label
0+0350 <[^>]*> addu \$a0,\$a0,\$gp
0+0354 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0354 R_MIPS_GOT_LO16 big_external_data_label
...
0+035c <[^>]*> addu \$a0,\$a0,\$a1
0+0360 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0360 R_MIPS_GOT_HI16 small_external_data_label
0+0364 <[^>]*> addu \$a0,\$a0,\$gp
0+0368 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0368 R_MIPS_GOT_LO16 small_external_data_label
...
0+0370 <[^>]*> addu \$a0,\$a0,\$a1
0+0374 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0374 R_MIPS_GOT_HI16 big_external_common
0+0378 <[^>]*> addu \$a0,\$a0,\$gp
0+037c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+037c R_MIPS_GOT_LO16 big_external_common
...
0+0384 <[^>]*> addu \$a0,\$a0,\$a1
0+0388 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0388 R_MIPS_GOT_HI16 small_external_common
0+038c <[^>]*> addu \$a0,\$a0,\$gp
0+0390 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0390 R_MIPS_GOT_LO16 small_external_common
...
0+0398 <[^>]*> addu \$a0,\$a0,\$a1
0+039c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+039c R_MIPS_GOT16 .bss
...
0+03a4 <[^>]*> addiu \$a0,\$a0,0
[ 	]*RELOC: 0+03a4 R_MIPS_LO16 .bss
0+03a8 <[^>]*> addu \$a0,\$a0,\$a1
0+03ac <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+03ac R_MIPS_GOT16 .bss
...
0+03b4 <[^>]*> addiu \$a0,\$a0,1000
[ 	]*RELOC: 0+03b4 R_MIPS_LO16 .bss
0+03b8 <[^>]*> addu \$a0,\$a0,\$a1
0+03bc <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+03bc R_MIPS_GOT16 .data
...
0+03c4 <[^>]*> addiu \$a0,\$a0,1
[ 	]*RELOC: 0+03c4 R_MIPS_LO16 .data
0+03c8 <[^>]*> addu \$a0,\$a0,\$a1
0+03cc <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+03cc R_MIPS_GOT_HI16 big_external_data_label
0+03d0 <[^>]*> addu \$a0,\$a0,\$gp
0+03d4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+03d4 R_MIPS_GOT_LO16 big_external_data_label
...
0+03dc <[^>]*> addiu \$a0,\$a0,1
0+03e0 <[^>]*> addu \$a0,\$a0,\$a1
0+03e4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+03e4 R_MIPS_GOT_HI16 small_external_data_label
0+03e8 <[^>]*> addu \$a0,\$a0,\$gp
0+03ec <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+03ec R_MIPS_GOT_LO16 small_external_data_label
...
0+03f4 <[^>]*> addiu \$a0,\$a0,1
0+03f8 <[^>]*> addu \$a0,\$a0,\$a1
0+03fc <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+03fc R_MIPS_GOT_HI16 big_external_common
0+0400 <[^>]*> addu \$a0,\$a0,\$gp
0+0404 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0404 R_MIPS_GOT_LO16 big_external_common
...
0+040c <[^>]*> addiu \$a0,\$a0,1
0+0410 <[^>]*> addu \$a0,\$a0,\$a1
0+0414 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0414 R_MIPS_GOT_HI16 small_external_common
0+0418 <[^>]*> addu \$a0,\$a0,\$gp
0+041c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+041c R_MIPS_GOT_LO16 small_external_common
...
0+0424 <[^>]*> addiu \$a0,\$a0,1
0+0428 <[^>]*> addu \$a0,\$a0,\$a1
0+042c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+042c R_MIPS_GOT16 .bss
...
0+0434 <[^>]*> addiu \$a0,\$a0,1
[ 	]*RELOC: 0+0434 R_MIPS_LO16 .bss
0+0438 <[^>]*> addu \$a0,\$a0,\$a1
0+043c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+043c R_MIPS_GOT16 .bss
...
0+0444 <[^>]*> addiu \$a0,\$a0,1001
[ 	]*RELOC: 0+0444 R_MIPS_LO16 .bss
0+0448 <[^>]*> addu \$a0,\$a0,\$a1
0+044c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+044c R_MIPS_GOT16 .data
0+0450 <[^>]*> lui \$at,0x1
0+0454 <[^>]*> addiu \$at,\$at,-32768
[ 	]*RELOC: 0+0454 R_MIPS_LO16 .data
0+0458 <[^>]*> addu \$a0,\$a0,\$at
0+045c <[^>]*> addu \$a0,\$a0,\$a1
0+0460 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0460 R_MIPS_GOT_HI16 big_external_data_label
0+0464 <[^>]*> addu \$a0,\$a0,\$gp
0+0468 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0468 R_MIPS_GOT_LO16 big_external_data_label
0+046c <[^>]*> lui \$at,0x1
0+0470 <[^>]*> addiu \$at,\$at,-32768
0+0474 <[^>]*> addu \$a0,\$a0,\$at
0+0478 <[^>]*> addu \$a0,\$a0,\$a1
0+047c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+047c R_MIPS_GOT_HI16 small_external_data_label
0+0480 <[^>]*> addu \$a0,\$a0,\$gp
0+0484 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0484 R_MIPS_GOT_LO16 small_external_data_label
0+0488 <[^>]*> lui \$at,0x1
0+048c <[^>]*> addiu \$at,\$at,-32768
0+0490 <[^>]*> addu \$a0,\$a0,\$at
0+0494 <[^>]*> addu \$a0,\$a0,\$a1
0+0498 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0498 R_MIPS_GOT_HI16 big_external_common
0+049c <[^>]*> addu \$a0,\$a0,\$gp
0+04a0 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+04a0 R_MIPS_GOT_LO16 big_external_common
0+04a4 <[^>]*> lui \$at,0x1
0+04a8 <[^>]*> addiu \$at,\$at,-32768
0+04ac <[^>]*> addu \$a0,\$a0,\$at
0+04b0 <[^>]*> addu \$a0,\$a0,\$a1
0+04b4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+04b4 R_MIPS_GOT_HI16 small_external_common
0+04b8 <[^>]*> addu \$a0,\$a0,\$gp
0+04bc <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+04bc R_MIPS_GOT_LO16 small_external_common
0+04c0 <[^>]*> lui \$at,0x1
0+04c4 <[^>]*> addiu \$at,\$at,-32768
0+04c8 <[^>]*> addu \$a0,\$a0,\$at
0+04cc <[^>]*> addu \$a0,\$a0,\$a1
0+04d0 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+04d0 R_MIPS_GOT16 .bss
0+04d4 <[^>]*> lui \$at,0x1
0+04d8 <[^>]*> addiu \$at,\$at,-32768
[ 	]*RELOC: 0+04d8 R_MIPS_LO16 .bss
0+04dc <[^>]*> addu \$a0,\$a0,\$at
0+04e0 <[^>]*> addu \$a0,\$a0,\$a1
0+04e4 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+04e4 R_MIPS_GOT16 .bss
0+04e8 <[^>]*> lui \$at,0x1
0+04ec <[^>]*> addiu \$at,\$at,-31768
[ 	]*RELOC: 0+04ec R_MIPS_LO16 .bss
0+04f0 <[^>]*> addu \$a0,\$a0,\$at
0+04f4 <[^>]*> addu \$a0,\$a0,\$a1
0+04f8 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+04f8 R_MIPS_GOT16 .data
...
0+0500 <[^>]*> addiu \$a0,\$a0,-32768
[ 	]*RELOC: 0+0500 R_MIPS_LO16 .data
0+0504 <[^>]*> addu \$a0,\$a0,\$a1
0+0508 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0508 R_MIPS_GOT_HI16 big_external_data_label
0+050c <[^>]*> addu \$a0,\$a0,\$gp
0+0510 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0510 R_MIPS_GOT_LO16 big_external_data_label
...
0+0518 <[^>]*> addiu \$a0,\$a0,-32768
0+051c <[^>]*> addu \$a0,\$a0,\$a1
0+0520 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0520 R_MIPS_GOT_HI16 small_external_data_label
0+0524 <[^>]*> addu \$a0,\$a0,\$gp
0+0528 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0528 R_MIPS_GOT_LO16 small_external_data_label
...
0+0530 <[^>]*> addiu \$a0,\$a0,-32768
0+0534 <[^>]*> addu \$a0,\$a0,\$a1
0+0538 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0538 R_MIPS_GOT_HI16 big_external_common
0+053c <[^>]*> addu \$a0,\$a0,\$gp
0+0540 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0540 R_MIPS_GOT_LO16 big_external_common
...
0+0548 <[^>]*> addiu \$a0,\$a0,-32768
0+054c <[^>]*> addu \$a0,\$a0,\$a1
0+0550 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0550 R_MIPS_GOT_HI16 small_external_common
0+0554 <[^>]*> addu \$a0,\$a0,\$gp
0+0558 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0558 R_MIPS_GOT_LO16 small_external_common
...
0+0560 <[^>]*> addiu \$a0,\$a0,-32768
0+0564 <[^>]*> addu \$a0,\$a0,\$a1
0+0568 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0568 R_MIPS_GOT16 .bss
...
0+0570 <[^>]*> addiu \$a0,\$a0,-32768
[ 	]*RELOC: 0+0570 R_MIPS_LO16 .bss
0+0574 <[^>]*> addu \$a0,\$a0,\$a1
0+0578 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0578 R_MIPS_GOT16 .bss
...
0+0580 <[^>]*> addiu \$a0,\$a0,-31768
[ 	]*RELOC: 0+0580 R_MIPS_LO16 .bss
0+0584 <[^>]*> addu \$a0,\$a0,\$a1
0+0588 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0588 R_MIPS_GOT16 .data
0+058c <[^>]*> lui \$at,0x1
0+0590 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+0590 R_MIPS_LO16 .data
0+0594 <[^>]*> addu \$a0,\$a0,\$at
0+0598 <[^>]*> addu \$a0,\$a0,\$a1
0+059c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+059c R_MIPS_GOT_HI16 big_external_data_label
0+05a0 <[^>]*> addu \$a0,\$a0,\$gp
0+05a4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+05a4 R_MIPS_GOT_LO16 big_external_data_label
0+05a8 <[^>]*> lui \$at,0x1
0+05ac <[^>]*> addiu \$at,\$at,0
0+05b0 <[^>]*> addu \$a0,\$a0,\$at
0+05b4 <[^>]*> addu \$a0,\$a0,\$a1
0+05b8 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+05b8 R_MIPS_GOT_HI16 small_external_data_label
0+05bc <[^>]*> addu \$a0,\$a0,\$gp
0+05c0 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+05c0 R_MIPS_GOT_LO16 small_external_data_label
0+05c4 <[^>]*> lui \$at,0x1
0+05c8 <[^>]*> addiu \$at,\$at,0
0+05cc <[^>]*> addu \$a0,\$a0,\$at
0+05d0 <[^>]*> addu \$a0,\$a0,\$a1
0+05d4 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+05d4 R_MIPS_GOT_HI16 big_external_common
0+05d8 <[^>]*> addu \$a0,\$a0,\$gp
0+05dc <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+05dc R_MIPS_GOT_LO16 big_external_common
0+05e0 <[^>]*> lui \$at,0x1
0+05e4 <[^>]*> addiu \$at,\$at,0
0+05e8 <[^>]*> addu \$a0,\$a0,\$at
0+05ec <[^>]*> addu \$a0,\$a0,\$a1
0+05f0 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+05f0 R_MIPS_GOT_HI16 small_external_common
0+05f4 <[^>]*> addu \$a0,\$a0,\$gp
0+05f8 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+05f8 R_MIPS_GOT_LO16 small_external_common
0+05fc <[^>]*> lui \$at,0x1
0+0600 <[^>]*> addiu \$at,\$at,0
0+0604 <[^>]*> addu \$a0,\$a0,\$at
0+0608 <[^>]*> addu \$a0,\$a0,\$a1
0+060c <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+060c R_MIPS_GOT16 .bss
0+0610 <[^>]*> lui \$at,0x1
0+0614 <[^>]*> addiu \$at,\$at,0
[ 	]*RELOC: 0+0614 R_MIPS_LO16 .bss
0+0618 <[^>]*> addu \$a0,\$a0,\$at
0+061c <[^>]*> addu \$a0,\$a0,\$a1
0+0620 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0620 R_MIPS_GOT16 .bss
0+0624 <[^>]*> lui \$at,0x1
0+0628 <[^>]*> addiu \$at,\$at,1000
[ 	]*RELOC: 0+0628 R_MIPS_LO16 .bss
0+062c <[^>]*> addu \$a0,\$a0,\$at
0+0630 <[^>]*> addu \$a0,\$a0,\$a1
0+0634 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0634 R_MIPS_GOT16 .data
0+0638 <[^>]*> lui \$at,0x2
0+063c <[^>]*> addiu \$at,\$at,-23131
[ 	]*RELOC: 0+063c R_MIPS_LO16 .data
0+0640 <[^>]*> addu \$a0,\$a0,\$at
0+0644 <[^>]*> addu \$a0,\$a0,\$a1
0+0648 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0648 R_MIPS_GOT_HI16 big_external_data_label
0+064c <[^>]*> addu \$a0,\$a0,\$gp
0+0650 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0650 R_MIPS_GOT_LO16 big_external_data_label
0+0654 <[^>]*> lui \$at,0x2
0+0658 <[^>]*> addiu \$at,\$at,-23131
0+065c <[^>]*> addu \$a0,\$a0,\$at
0+0660 <[^>]*> addu \$a0,\$a0,\$a1
0+0664 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0664 R_MIPS_GOT_HI16 small_external_data_label
0+0668 <[^>]*> addu \$a0,\$a0,\$gp
0+066c <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+066c R_MIPS_GOT_LO16 small_external_data_label
0+0670 <[^>]*> lui \$at,0x2
0+0674 <[^>]*> addiu \$at,\$at,-23131
0+0678 <[^>]*> addu \$a0,\$a0,\$at
0+067c <[^>]*> addu \$a0,\$a0,\$a1
0+0680 <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+0680 R_MIPS_GOT_HI16 big_external_common
0+0684 <[^>]*> addu \$a0,\$a0,\$gp
0+0688 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+0688 R_MIPS_GOT_LO16 big_external_common
0+068c <[^>]*> lui \$at,0x2
0+0690 <[^>]*> addiu \$at,\$at,-23131
0+0694 <[^>]*> addu \$a0,\$a0,\$at
0+0698 <[^>]*> addu \$a0,\$a0,\$a1
0+069c <[^>]*> lui \$a0,0x0
[ 	]*RELOC: 0+069c R_MIPS_GOT_HI16 small_external_common
0+06a0 <[^>]*> addu \$a0,\$a0,\$gp
0+06a4 <[^>]*> lw \$a0,0\(\$a0\)
[ 	]*RELOC: 0+06a4 R_MIPS_GOT_LO16 small_external_common
0+06a8 <[^>]*> lui \$at,0x2
0+06ac <[^>]*> addiu \$at,\$at,-23131
0+06b0 <[^>]*> addu \$a0,\$a0,\$at
0+06b4 <[^>]*> addu \$a0,\$a0,\$a1
0+06b8 <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+06b8 R_MIPS_GOT16 .bss
0+06bc <[^>]*> lui \$at,0x2
0+06c0 <[^>]*> addiu \$at,\$at,-23131
[ 	]*RELOC: 0+06c0 R_MIPS_LO16 .bss
0+06c4 <[^>]*> addu \$a0,\$a0,\$at
0+06c8 <[^>]*> addu \$a0,\$a0,\$a1
0+06cc <[^>]*> lw \$a0,0\(\$gp\)
[ 	]*RELOC: 0+06cc R_MIPS_GOT16 .bss
0+06d0 <[^>]*> lui \$at,0x2
0+06d4 <[^>]*> addiu \$at,\$at,-22131
[ 	]*RELOC: 0+06d4 R_MIPS_LO16 .bss
0+06d8 <[^>]*> addu \$a0,\$a0,\$at
0+06dc <[^>]*> addu \$a0,\$a0,\$a1
