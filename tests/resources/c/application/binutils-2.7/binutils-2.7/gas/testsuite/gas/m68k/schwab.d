#name: PC-relative relocations
#objdump: -drs -j .text

.*:     file format .*

Contents of section .text:
 0000 4e714e71 4cfa0300 fffa4cfa 0300fff4  NqNqL.....L.....
 0010 4cfb0300 08ee41fa ffea41fa ffe641fa  L.....A...A...A.
 0020 ff6241fb 08de41fb 08da41fb 08d641fb  .bA...A...A...A.
 0030 0920ffd2 41fb0920 ffcc41fb 0930ffff  . ..A.. ..A..0..
 0040 ffc641fb 0930ffff ffbe4e71 61ff0000  ..A..0....Nqa...
 0050 00586100 0052614e 614c4e71 41f90000  .Xa..RaNaLNqA...
 0060 00a641fa 004241fa 00be41fb 083a41fb  ..A..BA...A..:A.
 0070 083641fb 083241fb 0920002e 41fb0920  .6A..2A.. ..A.. 
 0080 002841fb 09300000 002241fb 09300000  .\(A..0..."A..0..
 0090 001a41fb 09300000 001241fb 0920000a  ..A..0....A.. ..
 00a0 41fb0804 4e714e71 4e7141fb 088041fb  A...NqNqNqA...A.
 00b0 0920ff7f 41fb0920 800041fb 0930ffff  . ..A.. ..A..0..
 00c0 7fff4e71 41fb087f 41fb0920 008041fb  ..NqA...A.. ..A.
 00d0 09207fff 41fb0930 00008000 4e7141fa  . ..A..0....NqA.
 00e0 800041fb 0170ffff 7fff4e71 41fa7fff  ..A..p....NqA...
 00f0 41fb0170 00008000 4e7141fb 0170ffff  A..p....NqA..p..
 0100 ff0441fb 0930ffff fefc4e71 41f90000  ..A..0....NqA...
 0110 0000...............................  ................
Disassembly of section \.text:
0+0000 <.*> nop
0+0002 <lbl_b> nop
0+0004 <lbl_b\+2> moveml 0+0002 <lbl_b>,%a0-%a1
0+000a <lbl_b\+8> moveml 0+0002 <lbl_b>,%a0-%a1
0+0010 <lbl_b\+e> moveml %pc@\(0+02 <lbl_b>,%d0:l\),%a0-%a1
0+0016 <lbl_b\+14> lea 0+0002 <lbl_b>,%a0
0+001a <lbl_b\+18> lea 0+0002 <lbl_b>,%a0
0+001e <lbl_b\+1c> lea f+ff82 <.*>,%a0
0+0022 <lbl_b\+20> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+0026 <lbl_b\+24> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+002a <lbl_b\+28> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+002e <lbl_b\+2c> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+0034 <lbl_b\+32> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+003a <lbl_b\+38> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+0042 <lbl_b\+40> lea %pc@\(0+02 <lbl_b>,%d0:l\),%a0
0+004a <lbl_b\+48> nop
0+004c <lbl_b\+4a> bsrl 0+00a6 <lbl_a>
0+0052 <lbl_b\+50> bsrw 0+00a6 <lbl_a>
0+0056 <lbl_b\+54> bsrs 0+00a6 <lbl_a>
0+0058 <lbl_b\+56> bsrs 0+00a6 <lbl_a>
0+005a <lbl_b\+58> nop
0+005c <lbl_b\+5a> lea 0+00a6 <lbl_a>,%a0
		RELOC: 0+005e (R_68K_)?32 \.text
0+0062 <lbl_b\+60> lea 0+00a6 <lbl_a>,%a0
0+0066 <lbl_b\+64> lea 0+0126 <.*>,%a0
0+006a <lbl_b\+68> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+006e <lbl_b\+6c> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+0072 <lbl_b\+70> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+0076 <lbl_b\+74> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+007c <lbl_b\+7a> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+0082 <lbl_b\+80> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+008a <lbl_b\+88> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+0092 <lbl_b\+90> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+009a <lbl_b\+98> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+00a0 <lbl_b\+9e> lea %pc@\(0+a6 <lbl_a>,%d0:l\),%a0
0+00a4 <lbl_b\+a2> nop
0+00a6 <lbl_a> nop
0+00a8 <lbl_a\+2> nop
0+00aa <lbl_a\+4> lea %pc@\(0+2c <lbl_b\+2a>,%d0:l\),%a0
0+00ae <lbl_a\+8> lea %pc@\(0+2f <lbl_b\+2d>,%d0:l\),%a0
0+00b4 <lbl_a\+e> lea %pc@\(f+80b6 <.*>,%d0:l\),%a0
0+00ba <lbl_a\+14> lea %pc@\(f+80bb <.*>,%d0:l\),%a0
0+00c2 <lbl_a\+1c> nop
0+00c4 <lbl_a\+1e> lea %pc@\(0+145 <.*>,%d0:l\),%a0
0+00c8 <lbl_a\+22> lea %pc@\(0+14a <.*>,%d0:l\),%a0
0+00ce <lbl_a\+28> lea %pc@\(0+80cf <.*>,%d0:l\),%a0
0+00d4 <lbl_a\+2e> lea %pc@\(0+80d6 <.*>,%d0:l\),%a0
0+00dc <lbl_a\+36> nop
0+00de <lbl_a\+38> lea f+80e0 <.*>,%a0
0+00e2 <lbl_a\+3c> lea %pc@\(f+80e3 <.*>\),%a0
0+00ea <lbl_a\+44> nop
0+00ec <lbl_a\+46> lea 0+80ed <.*>,%a0
0+00f0 <lbl_a\+4a> lea %pc@\(0+80f2 <.*>\),%a0
0+00f8 <lbl_a\+52> nop
0+00fa <lbl_a\+54> lea %pc@\(0+0 <.*>\),%a0
		RELOC: 0+00fe (R_68K_PC|DISP)32 undef
0+0102 <lbl_a\+5c> lea %pc@\(0+0 <.*>,%d0:l\),%a0
		RELOC: 0+0106 (R_68K_PC|DISP)32 undef
0+010a <lbl_a\+64> nop
0+010c <lbl_a\+66> lea 0+0+ <.*>,%a0
		RELOC: 0+010e (R_68K_)?32 undef
0+0112 <lbl_a\+6c> nop
\.\.\.
