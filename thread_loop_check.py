HASH_SIZE = 8
length = 1728
width = 1233

## goes from 0-63
MYTHREAD = 17

c1 = 0
while( c1 < length):
    c2 = 0
    while( c2 < width):
        first = length/HASH_SIZE * c1 / HASH_SIZE
        second = c2/HASH_SIZE
        ## GOES FROM 0-63
        whos_thread_this_belongs_to = (first + second) % 64
        ## SO IF THREAD_PROCESS == MYTHREAD
        if(whos_thread_this_belongs_to == MYTHREAD):
            do_the_thing()
        c2 += HASH_SIZE
    c1 += HASH_SIZE


def do_the_thing():
    four = 2 + 2
    print "Quick mafs"
