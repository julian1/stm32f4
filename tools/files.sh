
# echo "args are $@" 

find "$@" -type f | grep -v old | egrep '*.c$|*.h$'


