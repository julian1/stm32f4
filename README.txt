

mar 2024.

  # for tooling,
  nix-shell ~/devel/nixos-config/examples/arm.nix

  # in 4 separate shells,
  # for serial
  rlwrap -a picocom -b 115200 /dev/ttyUSB0
  # with new line.
  rlwrap -a picocom  -q -b 115200  --imap crcrlf   /dev/ttyUSB0

  # to make
  make

  # for st-link
  cd minimal
  openocd -f ../../openocd.cfg

  # for st-link comms
  rlwrap nc localhost 4444


  # write mcu firmware
  > reset halt ; flash write_image erase unlock /home/me/devel/stm32f4/projects/minimal/main.elf ; reset run

  # fpga firmware
  > reset halt; flash write_image erase unlock /home/me/devel/ice40-fpga/projects/minimal/build/main.bin.hdr  0x08060000 ; reset run


libopencm3 here,
 ../../lib/libopencm3




OLD.

quick start,

nix-shell ~/devel/nixos-config/examples/arm.nix  -I nixpkgs=/home/me/devel/nixpkgs02/


# build libopencm3. needed to generate headers

git clone git@github.com:libopencm3/libopencm3
nix-shell arm.nix etc.
cd libopencm3
make

# or git submodules init for libopencm3 etc.

git submodule update --init --recursive

cd libopencm3
make
ccd ..

# serial connection

rlwrap -a picocom  -q -b 115200 /dev/ttyUSB0
or
rlwrap -a picocom  -q -b 115200 /dev/ttyUSB0 | tee out.log
etc

(ctrl-c ctrl-c  to exit)

or
screen /dev/ttyUSB0 115200
(ctrl-a ctrk-k to exit)
or
picocom --baud 115200 /dev/ttyUSB0
(ctrl-a ctrk-x to exit)

etc.



cd projects/xxx
make clobber
make


# stlink

# actually, better in separate windows, avoids accidently swapping wrong context
openocd -f openocd.cfg
rlwrap nc localhost 4444
> reset
> halt
> reset halt ; flash write_image erase unlock ./projects/smu07/main.elf; reset run
> reset halt ; flash write_image erase unlock ./main.elf; sleep 1000; reset run
etc


