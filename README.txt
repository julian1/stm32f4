
quick start,


# build libopencm3. needed to generate headers

git clone git@github.com:libopencm3/libopencm3
nix-shell arm.nix etc.
cd libopencm3
make

# or git submodules init for libopencm3 etc.


# eg. uart
screen /dev/ttyUSB0 115200

cd projects/xxx
make clobber
make


# stlink
# actually, better in separate windows, avoid accidently swapping
openocd -f openocd.cfg
rlwrap nc localhost 4444
> reset
> halt
> reset halt ; flash write_image erase unlock ./projects/smu07/main.elf; reset run
> reset halt ; flash write_image erase unlock ./main.elf; sleep 1000; reset run
etc


