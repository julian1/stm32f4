

sed -i  's/CBuf/cbuf_t/' $1


sed -i  's/cBufInit/cbuf_init/' $1
sed -i  's/cBufisEmpty/cbuf_is_empty/' $1
sed -i  's/cBufCount/cbuf_count/' $1
sed -i  's/cBufReserve/cbuf_reserve/' $1
sed -i  's/cBufPeekLast/cbuf_peek_last/' $1
sed -i  's/cBufPeekFirst/cbuf_peek_first/' $1


sed -i  's/cBufClear/cbuf_clear/' $1
sed -i  's/cBufPush/cbuf_push/' $1
sed -i  's/cBufPop/cbuf_pop/' $1

sed -i  's/cBufCopyString/cbuf_copy_string/' $1
sed -i  's/cBufCopyString2/cbuf_copy_string2/' $1


sed -i  's/cBufRead/cbuf_read/' $1
sed -i  's/cBufWrite/cbuf_write/' $1


