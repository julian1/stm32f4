
quick start,


# build libopencm3. needed to generate headers

git clone git@github.com:libopencm3/libopencm3
nix-shell arm.nix etc.
cd libopencm3
make

# or git submodules init for libopencm3 etc.


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


