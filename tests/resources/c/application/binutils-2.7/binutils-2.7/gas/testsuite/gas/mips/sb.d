#objdump: -dr
#name: MIPS sb
#as: -mips1

# Test the sb macro.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> sb \$a0,0\(\$zero\)
0+0004 <[^>]*> sb \$a0,1\(\$zero\)
0+0008 <[^>]*> lui \$at,0x1
0+000c <[^>]*> sb \$a0,-32768\(\$at\)
0+0010 <[^>]*> sb \$a0,-32768\(\$zero\)
0+0014 <[^>]*> lui \$at,0x1
0+0018 <[^>]*> sb \$a0,0\(\$at\)
0+001c <[^>]*> lui \$at,0x2
0+0020 <[^>]*> sb \$a0,-23131\(\$at\)
0+0024 <[^>]*> sb \$a0,0\(\$a1\)
0+0028 <[^>]*> sb \$a0,1\(\$a1\)
0+002c <[^>]*> lui \$at,0x1
0+0030 <[^>]*> addu \$at,\$at,\$a1
0+0034 <[^>]*> sb \$a0,-32768\(\$at\)
0+0038 <[^>]*> sb \$a0,-32768\(\$a1\)
0+003c <[^>]*> lui \$at,0x1
0+0040 <[^>]*> addu \$at,\$at,\$a1
0+0044 <[^>]*> sb \$a0,0\(\$at\)
0+0048 <[^>]*> lui \$at,0x2
0+004c <[^>]*> addu \$at,\$at,\$a1
0+0050 <[^>]*> sb \$a0,-23131\(\$at\)
0+0054 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0054 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0058 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0058 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+005c <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+005c [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+0060 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+0060 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0064 <[^>]*> sb \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0064 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+0068 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0068 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+006c <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+006c [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+0070 <[^>]*> sb \$a0,0\(\$gp\)
[ 	]*RELOC: 0+0070 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+0074 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0074 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0078 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0078 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+007c <[^>]*> sb \$a0,[-0-9]+\(\$gp\)
[ 	]*RELOC: 0+007c [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+0080 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0080 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0084 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0084 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+0088 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0088 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+008c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+008c [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0090 <[^>]*> sb \$a0,1\(\$gp\)
[ 	]*RELOC: 0+0090 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+0094 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0094 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0098 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0098 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+009c <[^>]*> sb \$a0,1\(\$gp\)
[ 	]*RELOC: 0+009c [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+00a0 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00a0 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+00a4 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00a4 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+00a8 <[^>]*> sb \$a0,[-0-9]+\(\$gp\)
[ 	]*RELOC: 0+00a8 [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+00ac <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00ac [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+00b0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00b0 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+00b4 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00b4 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+00b8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00b8 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+00bc <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00bc [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+00c0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00c0 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+00c4 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00c4 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+00c8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00c8 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+00cc <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00cc [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+00d0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00d0 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+00d4 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00d4 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+00d8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00d8 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+00dc <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+00dc [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+00e0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00e0 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+00e4 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00e4 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+00e8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00e8 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+00ec <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00ec [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+00f0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00f0 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+00f4 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00f4 [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+00f8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+00f8 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+00fc <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+00fc [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0100 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0100 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+0104 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0104 [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+0108 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0108 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+010c <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+010c [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0110 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0110 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+0114 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0114 [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0118 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0118 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+011c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+011c [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0120 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0120 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+0124 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0124 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+0128 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+0128 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+012c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+012c [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+0130 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+0130 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+0134 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0134 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0138 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+0138 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+013c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+013c [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+0140 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+0140 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+0144 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0144 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0148 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0148 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+014c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+014c [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0150 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0150 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+0154 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0154 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0158 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0158 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+015c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+015c [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+0160 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0160 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0164 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0164 [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+0168 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0168 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+016c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+016c [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0170 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0170 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+0174 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0174 [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+0178 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0178 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+017c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+017c [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0180 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0180 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+0184 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0184 [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0188 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0188 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+018c <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+018c [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0190 <[^>]*> addu \$at,\$at,\$a1
0+0194 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0194 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+0198 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0198 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+019c <[^>]*> addu \$at,\$at,\$a1
0+01a0 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+01a0 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+01a4 <[^>]*> addu \$at,\$a1,\$gp
0+01a8 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+01a8 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+01ac <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+01ac [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+01b0 <[^>]*> addu \$at,\$at,\$a1
0+01b4 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+01b4 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+01b8 <[^>]*> addu \$at,\$a1,\$gp
0+01bc <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+01bc [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+01c0 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+01c0 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+01c4 <[^>]*> addu \$at,\$at,\$a1
0+01c8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+01c8 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+01cc <[^>]*> addu \$at,\$a1,\$gp
0+01d0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+01d0 [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+01d4 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+01d4 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+01d8 <[^>]*> addu \$at,\$at,\$a1
0+01dc <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+01dc [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+01e0 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+01e0 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+01e4 <[^>]*> addu \$at,\$at,\$a1
0+01e8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+01e8 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+01ec <[^>]*> addu \$at,\$a1,\$gp
0+01f0 <[^>]*> sb \$a0,1\(\$at\)
[ 	]*RELOC: 0+01f0 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+01f4 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+01f4 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+01f8 <[^>]*> addu \$at,\$at,\$a1
0+01fc <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+01fc [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+0200 <[^>]*> addu \$at,\$a1,\$gp
0+0204 <[^>]*> sb \$a0,1\(\$at\)
[ 	]*RELOC: 0+0204 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+0208 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0208 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+020c <[^>]*> addu \$at,\$at,\$a1
0+0210 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0210 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+0214 <[^>]*> addu \$at,\$a1,\$gp
0+0218 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0218 [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+021c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+021c [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0220 <[^>]*> addu \$at,\$at,\$a1
0+0224 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0224 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+0228 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0228 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+022c <[^>]*> addu \$at,\$at,\$a1
0+0230 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0230 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0234 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0234 [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+0238 <[^>]*> addu \$at,\$at,\$a1
0+023c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+023c [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+0240 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0240 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0244 <[^>]*> addu \$at,\$at,\$a1
0+0248 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0248 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+024c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+024c [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+0250 <[^>]*> addu \$at,\$at,\$a1
0+0254 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0254 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+0258 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0258 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+025c <[^>]*> addu \$at,\$at,\$a1
0+0260 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0260 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+0264 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0264 [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0268 <[^>]*> addu \$at,\$at,\$a1
0+026c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+026c [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+0270 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0270 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+0274 <[^>]*> addu \$at,\$at,\$a1
0+0278 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0278 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+027c <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+027c [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+0280 <[^>]*> addu \$at,\$at,\$a1
0+0284 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0284 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0288 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0288 [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+028c <[^>]*> addu \$at,\$at,\$a1
0+0290 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0290 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+0294 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+0294 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0298 <[^>]*> addu \$at,\$at,\$a1
0+029c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+029c [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+02a0 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+02a0 [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+02a4 <[^>]*> addu \$at,\$at,\$a1
0+02a8 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+02a8 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+02ac <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+02ac [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+02b0 <[^>]*> addu \$at,\$at,\$a1
0+02b4 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+02b4 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+02b8 <[^>]*> lui \$at,0x0
[ 	]*RELOC: 0+02b8 [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+02bc <[^>]*> addu \$at,\$at,\$a1
0+02c0 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+02c0 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+02c4 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+02c4 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+02c8 <[^>]*> addu \$at,\$at,\$a1
0+02cc <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+02cc [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+02d0 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+02d0 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+02d4 <[^>]*> addu \$at,\$at,\$a1
0+02d8 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+02d8 [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+02dc <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+02dc [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+02e0 <[^>]*> addu \$at,\$at,\$a1
0+02e4 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+02e4 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+02e8 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+02e8 [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+02ec <[^>]*> addu \$at,\$at,\$a1
0+02f0 <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+02f0 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+02f4 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+02f4 [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+02f8 <[^>]*> addu \$at,\$at,\$a1
0+02fc <[^>]*> sb \$a0,0\(\$at\)
[ 	]*RELOC: 0+02fc [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+0300 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0300 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0304 <[^>]*> addu \$at,\$at,\$a1
0+0308 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0308 [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+030c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+030c [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0310 <[^>]*> addu \$at,\$at,\$a1
0+0314 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0314 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+0318 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0318 [A-Z0-9_]*HI[A-Z0-9_]* .data.*
0+031c <[^>]*> addu \$at,\$at,\$a1
0+0320 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0320 [A-Z0-9_]*LO[A-Z0-9_]* .data.*
0+0324 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0324 [A-Z0-9_]*HI[A-Z0-9_]* big_external_data_label
0+0328 <[^>]*> addu \$at,\$at,\$a1
0+032c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+032c [A-Z0-9_]*LO[A-Z0-9_]* big_external_data_label
0+0330 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0330 [A-Z0-9_]*HI[A-Z0-9_]* small_external_data_label
0+0334 <[^>]*> addu \$at,\$at,\$a1
0+0338 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0338 [A-Z0-9_]*LO[A-Z0-9_]* small_external_data_label
0+033c <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+033c [A-Z0-9_]*HI[A-Z0-9_]* big_external_common
0+0340 <[^>]*> addu \$at,\$at,\$a1
0+0344 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0344 [A-Z0-9_]*LO[A-Z0-9_]* big_external_common
0+0348 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0348 [A-Z0-9_]*HI[A-Z0-9_]* small_external_common
0+034c <[^>]*> addu \$at,\$at,\$a1
0+0350 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0350 [A-Z0-9_]*LO[A-Z0-9_]* small_external_common
0+0354 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0354 [A-Z0-9_]*HI[A-Z0-9_]* .bss.*
0+0358 <[^>]*> addu \$at,\$at,\$a1
0+035c <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+035c [A-Z0-9_]*LO[A-Z0-9_]* .bss.*
0+0360 <[^>]*> lui \$at,[-0-9x]+
[ 	]*RELOC: 0+0360 [A-Z0-9_]*HI[A-Z0-9_]* .sbss.*
0+0364 <[^>]*> addu \$at,\$at,\$a1
0+0368 <[^>]*> sb \$a0,[-0-9]+\(\$at\)
[ 	]*RELOC: 0+0368 [A-Z0-9_]*LO[A-Z0-9_]* .sbss.*
0+036c <[^>]*> sw \$a0,0\(\$zero\)
0+0370 <[^>]*> sw \$a1,4\(\$zero\)
0+0374 <[^>]*> sh \$a0,0\(\$zero\)
0+0378 <[^>]*> sw \$a0,0\(\$zero\)
0+037c <[^>]*> sc \$a0,0\(\$zero\)
0+0380 <[^>]*> swc1 \$f4,0\(\$zero\)
0+0384 <[^>]*> swc2 \$4,0\(\$zero\)
0+0388 <[^>]*> swc3 \$4,0\(\$zero\)
0+038c <[^>]*> swc1 \$f4,0\(\$zero\)
0+0390 <[^>]*> swl \$a0,0\(\$zero\)
0+0394 <[^>]*> swr \$a0,0\(\$zero\)
...
