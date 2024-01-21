
set -e
set -x


pushd ../../lib/lib2/; make clobber ; make ; popd
pushd ../../lib/lzo/; make clobber ; make ; popd
make clobber ; make 



