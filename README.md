genfileio
=========

This is a file-based benchmar which is doing some fixed-block size IOs on a file with random SEEKs.

Usage:
======

    $ make
    gcc -o genfileio genfileio.c
    $ dd if=/dev/urandom of=test.raw bs=1M count=100
    100+0 records in
    100+0 records out
    104857600 bytes (105 MB) copied, 5.7305 s, 18.3 MB/s
    $ ./genfileio -f ./test.raw -b 128
    [-] Going to use ./test.raw with 128K blocks..
    [-] File opened succesfully, going to read/write randomly forever now...
    [-] File size: 104857600 bytes
        [-READ] 4 MB in 1 seconds
        [WRITE] 4 MB in 1 seconds
        [-READ] 15 MB in 1 seconds
        [WRITE] 15 MB in 1 seconds
        [-READ] 8 MB in 1 seconds
        [WRITE] 8 MB in 1 seconds
