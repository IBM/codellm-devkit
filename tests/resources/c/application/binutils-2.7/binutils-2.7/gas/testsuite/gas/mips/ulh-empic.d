#objdump: -dr
#name: MIPS ulh-empic
#as: -mips1 -membedded-pic
#source: ulh-pic.s

# Test the ulh macro with -membedded-pic.

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> addiu \$at,\$gp,-16384
[ 	]*RELOC: 0+0000 [A-Z0-9_]*GPREL[A-Z0-9_]* .sdata.*
0+0004 <[^>]*> lb \$a0,[01]\(\$at\)
0+0008 <[^>]*> lbu \$at,[01]\(\$at\)
0+000c <[^>]*> sll \$a0,\$a0,0x8
0+0010 <[^>]*> or \$a0,\$a0,\$at
0+0014 <[^>]*> addiu \$at,\$gp,0
[ 	]*RELOC: 0+0014 [A-Z0-9_]*GPREL[A-Z0-9_]* big_external_data_label
0+0018 <[^>]*> lbu \$a0,[01]\(\$at\)
0+001c <[^>]*> lbu \$at,[01]\(\$at\)
0+0020 <[^>]*> sll \$a0,\$a0,0x8
0+0024 <[^>]*> or \$a0,\$a0,\$at
0+0028 <[^>]*> addiu \$at,\$gp,0
[ 	]*RELOC: 0+0028 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+002c <[^>]*> lwl \$a0,[03]\(\$at\)
0+0030 <[^>]*> lwr \$a0,[03]\(\$at\)
0+0034 <[^>]*> addiu \$at,\$gp,0
[ 	]*RELOC: 0+0034 [A-Z0-9_]*GPREL[A-Z0-9_]* big_external_common
0+0038 <[^>]*> sb \$a0,[01]\(\$at\)
0+003c <[^>]*> srl \$a0,\$a0,0x8
0+0040 <[^>]*> sb \$a0,[01]\(\$at\)
0+0044 <[^>]*> lbu \$at,[01]\(\$at\)
0+0048 <[^>]*> sll \$a0,\$a0,0x8
0+004c <[^>]*> or \$a0,\$a0,\$at
0+0050 <[^>]*> addiu \$at,\$gp,0
[ 	]*RELOC: 0+0050 [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+0054 <[^>]*> swl \$a0,[03]\(\$at\)
0+0058 <[^>]*> swr \$a0,[03]\(\$at\)
0+005c <[^>]*> addiu \$at,\$gp,-16384
[ 	]*RELOC: 0+005c [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+0060 <[^>]*> lb \$a0,[01]\(\$at\)
0+0064 <[^>]*> lbu \$at,[01]\(\$at\)
0+0068 <[^>]*> sll \$a0,\$a0,0x8
0+006c <[^>]*> or \$a0,\$a0,\$at
0+0070 <[^>]*> addiu \$at,\$gp,-15384
[ 	]*RELOC: 0+0070 [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+0074 <[^>]*> lbu \$a0,[01]\(\$at\)
0+0078 <[^>]*> lbu \$at,[01]\(\$at\)
0+007c <[^>]*> sll \$a0,\$a0,0x8
0+0080 <[^>]*> or \$a0,\$a0,\$at
0+0084 <[^>]*> addiu \$at,\$gp,-16383
[ 	]*RELOC: 0+0084 [A-Z0-9_]*GPREL[A-Z0-9_]* .sdata.*
0+0088 <[^>]*> lwl \$a0,[03]\(\$at\)
0+008c <[^>]*> lwr \$a0,[03]\(\$at\)
0+0090 <[^>]*> addiu \$at,\$gp,1
[ 	]*RELOC: 0+0090 [A-Z0-9_]*GPREL[A-Z0-9_]* big_external_data_label
0+0094 <[^>]*> sb \$a0,[01]\(\$at\)
0+0098 <[^>]*> srl \$a0,\$a0,0x8
0+009c <[^>]*> sb \$a0,[01]\(\$at\)
0+00a0 <[^>]*> lbu \$at,[01]\(\$at\)
0+00a4 <[^>]*> sll \$a0,\$a0,0x8
0+00a8 <[^>]*> or \$a0,\$a0,\$at
0+00ac <[^>]*> addiu \$at,\$gp,1
[ 	]*RELOC: 0+00ac [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_data_label
0+00b0 <[^>]*> swl \$a0,[03]\(\$at\)
0+00b4 <[^>]*> swr \$a0,[03]\(\$at\)
0+00b8 <[^>]*> addiu \$at,\$gp,1
[ 	]*RELOC: 0+00b8 [A-Z0-9_]*GPREL[A-Z0-9_]* big_external_common
0+00bc <[^>]*> lb \$a0,[01]\(\$at\)
0+00c0 <[^>]*> lbu \$at,[01]\(\$at\)
0+00c4 <[^>]*> sll \$a0,\$a0,0x8
0+00c8 <[^>]*> or \$a0,\$a0,\$at
0+00cc <[^>]*> addiu \$at,\$gp,1
[ 	]*RELOC: 0+00cc [A-Z0-9_]*GPREL[A-Z0-9_]* small_external_common
0+00d0 <[^>]*> lbu \$a0,[01]\(\$at\)
0+00d4 <[^>]*> lbu \$at,[01]\(\$at\)
0+00d8 <[^>]*> sll \$a0,\$a0,0x8
0+00dc <[^>]*> or \$a0,\$a0,\$at
0+00e0 <[^>]*> addiu \$at,\$gp,-16383
[ 	]*RELOC: 0+00e0 [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+00e4 <[^>]*> lwl \$a0,[03]\(\$at\)
0+00e8 <[^>]*> lwr \$a0,[03]\(\$at\)
0+00ec <[^>]*> addiu \$at,\$gp,-15383
[ 	]*RELOC: 0+00ec [A-Z0-9_]*GPREL[A-Z0-9_]* .sbss.*
0+00f0 <[^>]*> sb \$a0,[01]\(\$at\)
0+00f4 <[^>]*> srl \$a0,\$a0,0x8
0+00f8 <[^>]*> sb \$a0,[01]\(\$at\)
0+00fc <[^>]*> lbu \$at,[01]\(\$at\)
0+0100 <[^>]*> sll \$a0,\$a0,0x8
0+0104 <[^>]*> or \$a0,\$a0,\$at
...
