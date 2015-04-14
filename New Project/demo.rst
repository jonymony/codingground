                              1 ;--------------------------------------------------------
                              2 ; File Created by SDCC : free open source ANSI-C Compiler
                              3 ; Version 3.3.0 #8604 (Jul 16 2014) (Linux)
                              4 ; This file was generated Tue Apr 14 05:38:51 2015
                              5 ;--------------------------------------------------------
                              6 	.module demo
                              7 	.optsdcc -mmcs51 --model-small
                              8 	
                              9 ;--------------------------------------------------------
                             10 ; Public variables in this module
                             11 ;--------------------------------------------------------
                             12 	.globl _main
                             13 	.globl _strcpy
                             14 ;--------------------------------------------------------
                             15 ; special function registers
                             16 ;--------------------------------------------------------
                             17 	.area RSEG    (ABS,DATA)
   0000                      18 	.org 0x0000
                             19 ;--------------------------------------------------------
                             20 ; special function bits
                             21 ;--------------------------------------------------------
                             22 	.area RSEG    (ABS,DATA)
   0000                      23 	.org 0x0000
                             24 ;--------------------------------------------------------
                             25 ; overlayable register banks
                             26 ;--------------------------------------------------------
                             27 	.area REG_BANK_0	(REL,OVR,DATA)
   0000                      28 	.ds 8
                             29 ;--------------------------------------------------------
                             30 ; internal ram data
                             31 ;--------------------------------------------------------
                             32 	.area DSEG    (DATA)
   0008                      33 _main_str_1_21:
   0008                      34 	.ds 20
                             35 ;--------------------------------------------------------
                             36 ; overlayable items in internal ram 
                             37 ;--------------------------------------------------------
                             38 ;--------------------------------------------------------
                             39 ; Stack segment in internal ram 
                             40 ;--------------------------------------------------------
                             41 	.area	SSEG	(DATA)
   0022                      42 __start__stack:
   0022                      43 	.ds	1
                             44 
                             45 ;--------------------------------------------------------
                             46 ; indirectly addressable internal ram data
                             47 ;--------------------------------------------------------
                             48 	.area ISEG    (DATA)
                             49 ;--------------------------------------------------------
                             50 ; absolute internal ram data
                             51 ;--------------------------------------------------------
                             52 	.area IABS    (ABS,DATA)
                             53 	.area IABS    (ABS,DATA)
                             54 ;--------------------------------------------------------
                             55 ; bit data
                             56 ;--------------------------------------------------------
                             57 	.area BSEG    (BIT)
                             58 ;--------------------------------------------------------
                             59 ; paged external ram data
                             60 ;--------------------------------------------------------
                             61 	.area PSEG    (PAG,XDATA)
                             62 ;--------------------------------------------------------
                             63 ; external ram data
                             64 ;--------------------------------------------------------
                             65 	.area XSEG    (XDATA)
                             66 ;--------------------------------------------------------
                             67 ; absolute external ram data
                             68 ;--------------------------------------------------------
                             69 	.area XABS    (ABS,XDATA)
                             70 ;--------------------------------------------------------
                             71 ; external initialized ram data
                             72 ;--------------------------------------------------------
                             73 	.area XISEG   (XDATA)
                             74 	.area HOME    (CODE)
                             75 	.area GSINIT0 (CODE)
                             76 	.area GSINIT1 (CODE)
                             77 	.area GSINIT2 (CODE)
                             78 	.area GSINIT3 (CODE)
                             79 	.area GSINIT4 (CODE)
                             80 	.area GSINIT5 (CODE)
                             81 	.area GSINIT  (CODE)
                             82 	.area GSFINAL (CODE)
                             83 	.area CSEG    (CODE)
                             84 ;--------------------------------------------------------
                             85 ; interrupt vector 
                             86 ;--------------------------------------------------------
                             87 	.area HOME    (CODE)
   0000                      88 __interrupt_vect:
   0000 02 00 06      [24]   89 	ljmp	__sdcc_gsinit_startup
                             90 ;--------------------------------------------------------
                             91 ; global & static initialisations
                             92 ;--------------------------------------------------------
                             93 	.area HOME    (CODE)
                             94 	.area GSINIT  (CODE)
                             95 	.area GSFINAL (CODE)
                             96 	.area GSINIT  (CODE)
                             97 	.globl __sdcc_gsinit_startup
                             98 	.globl __sdcc_program_startup
                             99 	.globl __start__stack
                            100 	.globl __mcs51_genXINIT
                            101 	.globl __mcs51_genXRAMCLEAR
                            102 	.globl __mcs51_genRAMCLEAR
                            103 	.area GSFINAL (CODE)
   005F 02 00 03      [24]  104 	ljmp	__sdcc_program_startup
                            105 ;--------------------------------------------------------
                            106 ; Home
                            107 ;--------------------------------------------------------
                            108 	.area HOME    (CODE)
                            109 	.area HOME    (CODE)
   0003                     110 __sdcc_program_startup:
   0003 02 00 62      [24]  111 	ljmp	_main
                            112 ;	return from main will return to caller
                            113 ;--------------------------------------------------------
                            114 ; code
                            115 ;--------------------------------------------------------
                            116 	.area CSEG    (CODE)
                            117 ;------------------------------------------------------------
                            118 ;Allocation info for local variables in function 'main'
                            119 ;------------------------------------------------------------
                            120 ;str                       Allocated with name '_main_str_1_21'
                            121 ;------------------------------------------------------------
                            122 ;	demo.c:3: int main()
                            123 ;	-----------------------------------------
                            124 ;	 function main
                            125 ;	-----------------------------------------
   0062                     126 _main:
                     0007   127 	ar7 = 0x07
                     0006   128 	ar6 = 0x06
                     0005   129 	ar5 = 0x05
                     0004   130 	ar4 = 0x04
                     0003   131 	ar3 = 0x03
                     0002   132 	ar2 = 0x02
                     0001   133 	ar1 = 0x01
                     0000   134 	ar0 = 0x00
                            135 ;	demo.c:7: strcpy(str, "Hello, Word!");
   0062 75 1C F3      [24]  136 	mov	_strcpy_PARM_2,#__str_0
   0065 75 1D 00      [24]  137 	mov	(_strcpy_PARM_2 + 1),#(__str_0 >> 8)
   0068 75 1E 80      [24]  138 	mov	(_strcpy_PARM_2 + 2),#0x80
   006B 90 00 08      [24]  139 	mov	dptr,#_main_str_1_21
   006E 75 F0 40      [24]  140 	mov	b,#0x40
   0071 12 00 78      [24]  141 	lcall	_strcpy
                            142 ;	demo.c:9: return 0;
   0074 90 00 00      [24]  143 	mov	dptr,#0x0000
   0077 22            [24]  144 	ret
                            145 	.area CSEG    (CODE)
                            146 	.area CONST   (CODE)
   00F3                     147 __str_0:
   00F3 48 65 6C 6C 6F 2C   148 	.ascii "Hello, Word!"
        20 57 6F 72 64 21
   00FF 00                  149 	.db 0x00
                            150 	.area XINIT   (CODE)
                            151 	.area CABS    (ABS,CODE)
