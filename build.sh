
set -e
set -x

# submodules may not be needed
# git submodule update --init --recursive
# libopencm3 is pinned?

# for i in [ ] etc.
pushd lib/libopencm3/;  make clean ; make ; popd
pushd lib/mesch12b/;    make clobber ; make ; popd
pushd lib/lib2/;        make clobber ; make ; popd
pushd lib/lzo/;         make clobber ; make ; popd

pushd lib/mongoose/;         make clean ; make ; popd

pushd projects/minimal; make clobber ; make ; popd


