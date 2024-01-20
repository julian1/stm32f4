
/*
  TODO
    increase mem etc.


[nix-shell:~/devel/stm32f4]$ st-info --probe
Found 1 stlink programmers
  version:    V2J29S7
  serial:     3E17030032124353354B4E00
  flash:      1048576 (pagesize: 16384)
  sram:       327680
  chipid:     0x0463
  descr:      F413


> flash banks
flash banks
#0 : stm32f4x.flash (stm32f2x) at 0x08000000, size 0x00100000, buswidth 0, chipwidth 0
#1 : stm32f4x.otp (stm32f2x) at 0x1fff7800, size 0x00000000, buswidth 0, chipwidth 0


> flash info 0
flash info 0
#0 : stm32f2x at 0x08000000, size 0x00100000, buswidth 0, chipwidth 0
        #  0: 0x00000000 (0x4000 16kB) not protected
        #  1: 0x00004000 (0x4000 16kB) not protected
        #  2: 0x00008000 (0x4000 16kB) not protected
        #  3: 0x0000c000 (0x4000 16kB) not protected
        #  4: 0x00010000 (0x10000 64kB) not protected
        #  5: 0x00020000 (0x20000 128kB) not protected
        #  6: 0x00040000 (0x20000 128kB) not protected
        #  7: 0x00060000 (0x20000 128kB) not protected
        #  8: 0x00080000 (0x20000 128kB) not protected
        #  9: 0x000a0000 (0x20000 128kB) not protected
        # 10: 0x000c0000 (0x20000 128kB) not protected
        # 11: 0x000e0000 (0x20000 128kB) not protected
        # 12: 0x00000000 (0x0 0kB) not protected
        # 13: 0xa7e9db20 (0x7f44 31kB) not protected
        # 14: 0x000000f0 (0x0 0kB) not protected
STM32F4[1|2]3 - Rev: A



*/

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 512K
	/* ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K */

	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 327680

}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld

