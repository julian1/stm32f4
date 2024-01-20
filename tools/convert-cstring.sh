


sed -i  's/CString/cstring_t/' $1



sed -i  's/cStringInit/cstring_init/' $1
sed -i  's/cStringisEmpty/cstring_empty/' $1
sed -i  's/cStringCount/cstring_count/' $1
sed -i  's/cStringReserve/cstring_reserve/' $1
sed -i  's/cStringPeekLast/cstring_peek_last/' $1
sed -i  's/cStringPeekFirst/cstring_peek_first/' $1
sed -i  's/cStringClear/cstring_clear/' $1
sed -i  's/cStringPush/cstring_push/' $1
sed -i  's/cStringPop/cstring_pop/' $1
sed -i  's/cStringPtr/cstring_ptr/' $1
sed -i  's/cStringWrite/cstring_write/' $1



