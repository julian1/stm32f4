
/*
f429ZIT  Z=144pin, I=2MB flash, T=lqfp

[nix-shell:~/devel/stm32f4]$ st-info --probe
Found 1 stlink programmers
  version:    V2J29S7
  serial:     4837060032124353354B4E00
  flash:      2097152 (pagesize: 16384)
  sram:       262144
  chipid:     0x0419
  descr:      F42x/F43x


> flash banks
flash banks
#0 : stm32f4x.flash (stm32f2x) at 0x00000000, size 0x00000000, buswidth 0, chipwidth 0
#1 : stm32f4x.otp (stm32f2x) at 0x1fff7800, size 0x00000000, buswidth 0, chipwidth 0

> flash info 0
flash info 0
device id = 0x20036419
flash size = 2048 KiB
Dual Bank 2048 kiB STM32F42x/43x/469/479 found
#0 : stm32f2x at 0x08000000, size 0x00200000, buswidth 0, chipwidth 0
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
        # 12: 0x00100000 (0x4000 16kB) not protected
        # 13: 0x00104000 (0x4000 16kB) not protected
        # 14: 0x00108000 (0x4000 16kB) not protected
        # 15: 0x0010c000 (0x4000 16kB) not protected
        # 16: 0x00110000 (0x10000 64kB) not protected
        # 17: 0x00120000 (0x20000 128kB) not protected
        # 18: 0x00140000 (0x20000 128kB) not protected
        # 19: 0x00160000 (0x20000 128kB) not protected
        # 20: 0x00180000 (0x20000 128kB) not protected
        # 21: 0x001a0000 (0x20000 128kB) not protected
        # 22: 0x001c0000 (0x20000 128kB) not protected
        # 23: 0x001e0000 (0x20000 128kB) not protected
STM32F4xx - Rev: unknown (0x2003)


*/


/* Define memory regions. */
MEMORY
{

  /* 256k ram doesn't work  - weird. but 128k does
    needs more decoupling?
   */
/*
  rom (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
*/

  /* see, https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/stm32f429i-discovery.ld
  */
  rom (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
  ccm (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 192K

}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld

