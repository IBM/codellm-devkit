#objdump: -dr
#name: MIPS div
#source: div.s

# Test the div macro.

.*: +file format .*mips.*

No symbols in .*
Disassembly of section .text:
0+0000 div \$zero,\$a0,\$a1
0+0004 div \$zero,\$a0,\$a1
0+0008 bnez \$a1,0+0014
...
0+0010 break 0x7
0+0014 li \$at,-1
0+0018 bne \$a1,\$at,0+002c
0+001c lui \$at,0x8000
0+0020 bne \$a0,\$at,0+002c
...
0+0028 break 0x6
0+002c mflo \$a0
0+0030 div \$zero,\$a1,\$a2
0+0034 bnez \$a2,0+0040
...
0+003c break 0x7
0+0040 li \$at,-1
0+0044 bne \$a2,\$at,0+0058
0+0048 lui \$at,0x8000
0+004c bne \$a1,\$at,0+0058
...
0+0054 break 0x6
0+0058 mflo \$a0
0+005c move \$a0,\$a0
0+0060 move \$a0,\$a1
0+0064 neg \$a0,\$a0
0+0068 neg \$a0,\$a1
0+006c li \$at,2
0+0070 div \$zero,\$a0,\$at
0+0074 mflo \$a0
0+0078 li \$at,2
0+007c div \$zero,\$a1,\$at
0+0080 mflo \$a0
0+0084 li \$at,0x8000
0+0088 div \$zero,\$a0,\$at
0+008c mflo \$a0
0+0090 li \$at,0x8000
0+0094 div \$zero,\$a1,\$at
0+0098 mflo \$a0
0+009c li \$at,-32768
0+00a0 div \$zero,\$a0,\$at
0+00a4 mflo \$a0
0+00a8 li \$at,-32768
0+00ac div \$zero,\$a1,\$at
0+00b0 mflo \$a0
0+00b4 lui \$at,0x1
0+00b8 div \$zero,\$a0,\$at
0+00bc mflo \$a0
0+00c0 lui \$at,0x1
0+00c4 div \$zero,\$a1,\$at
0+00c8 mflo \$a0
0+00cc lui \$at,0x1
0+00d0 ori \$at,\$at,0xa5a5
0+00d4 div \$zero,\$a0,\$at
0+00d8 mflo \$a0
0+00dc lui \$at,0x1
0+00e0 ori \$at,\$at,0xa5a5
0+00e4 div \$zero,\$a1,\$at
0+00e8 mflo \$a0
0+00ec divu \$zero,\$a0,\$a1
0+00f0 divu \$zero,\$a0,\$a1
0+00f4 bnez \$a1,0+0100
...
0+00fc break 0x7
0+0100 mflo \$a0
0+0104 divu \$zero,\$a1,\$a2
0+0108 bnez \$a2,0+0114
...
0+0110 break 0x7
0+0114 mflo \$a0
0+0118 move \$a0,\$a0
0+011c div \$zero,\$a1,\$a2
0+0120 bnez \$a2,0+012c
...
0+0128 break 0x7
0+012c li \$at,-1
0+0130 bne \$a2,\$at,0+0144
0+0134 lui \$at,0x8000
0+0138 bne \$a1,\$at,0+0144
...
0+0140 break 0x6
0+0144 mfhi \$a0
0+0148 li \$at,2
0+014c divu \$zero,\$a1,\$at
0+0150 mfhi \$a0
0+0154 ddiv \$zero,\$a1,\$a2
0+0158 bnez \$a2,0+0164
...
0+0160 break 0x7
0+0164 daddiu \$at,\$zero,-1
0+0168 bne \$a2,\$at,0+0180
0+016c daddiu \$at,\$zero,1
0+0170 dsll32 \$at,\$at,0x1f
0+0174 bne \$a1,\$at,0+0180
...
0+017c break 0x6
0+0180 mflo \$a0
0+0184 li \$at,2
0+0188 ddivu \$zero,\$a1,\$at
0+018c mflo \$a0
0+0190 li \$at,0x8000
0+0194 ddiv \$zero,\$a1,\$at
0+0198 mfhi \$a0
0+019c li \$at,-32768
0+01a0 ddivu \$zero,\$a1,\$at
0+01a4 mfhi \$a0
...
