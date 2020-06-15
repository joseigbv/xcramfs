# xcramfs

Extract cram filesystem with lzma compression for debug or forensic. 

CramFS + LZMA, used by many home routers in their firmware.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Tools can be compiled on any Win32 or UNIX/Linux box. You only need a relatively modern C compiler.

### Installing

Download a copy of the project from github:

```
$ git clone https://github.com/joseigbv/xcramfs.git
```

Edit 'search.c' and set firmware file and signature bytes:

...
#define FNAME "openrg.rta9211w_6_0_18_1_11.img"
const char PAT[] = { 0x45, 0x3d, 0xcd, 0x28 };
...

Compile:

```
$ gcc -Wall -O2 search.c -o search
```

Edit 'xcramfs.c' and change configuration (optional):

```
...
#define FNAME "data.cramfs"
#define BASE "x"
...
```

Compile:

```
$ cd LZMA_C && make && cd ..
$ gcc -m32 -Wall -ILZMA_C xcramfs.c LZMA_C/decode.o LZMA_C/LzmaDecode.o -o xcramfs 
```

### Usage

First, you need to find and extract the cramfs with lzma compression: 

```
$ ./search

Searching for CRAMFS filesystem...
* Pattern found in byte 1490944.

Struct Super:
=============

Magic:          0x28cd3d45
Size:           0x00350000
Flags:          0x0000a003
Future:         0x00000000
Signature:      C o m p r e s s e d   R O M F S
Fsid:           0x08 0x18 0x79 0x2b 0x00 0x00 0x00 0x00 0x0a 0x02 0x00 0x00 0xa4 0x01 0x00 0x00
Name:           0x08 0x18 0x79 0x2b 0x00 0x00 0x00 0x00 0x0a 0x02 0x00 0x00 0xa4 0x01 0x00 0x00


# 1490944 / 512 = 2912 
# 0x00350000 / 512 = 6784
$ dd if=openrg.rta9211w_6_0_18_1_11.img of=data.cramfs skip=2912 count=6784

Extract the filesystem:

```
$ ./xcramfs 

*** cramfs test ***

struct super:
=============

        magic:          0x28cd3d45
        size:           0x  350000
        flags:          0x    a003
        future:         0x       0
        signature:      C o m p r e s s e d   R O M F S
        fsid:           0x 8 0x18 0x79 0x2b 0x 0 0x 0 0x 0 0x 0 0x a 0x 2 0x 0 0x 0 0xa4 0x 1 0x 0 0x 0
        name:           0x 8 0x18 0x79 0x2b 0x 0 0x 0 0x 0 0x 0 0x a 0x 2 0x 0 0x 0 0xa4 0x 1 0x 0 0x 0


x/bin (dir)
x/bin/adslctrl (file)
x/bin/busybox (file)
x/bin/dhcp6c (file)
x/bin/dhcp6s (file)
...

```

Create library links:

# ln-libs.sh 

We can now use qemu to launch and debug the binaries (e.g.)

```
$ chroot ./x qemu-mips-static -g 12345 /bin/busybox
```

And:

```
$ mips-linux-gnu-gdb ./x/bin/busybox
(gdb) target remote 127.0.0.1:12345
(gdb) info functions
(gdb) break main 
(gdb) cont 
(gdb) disassemble main
...
```

## References

* http://mercawebonline.blogspot.com.es/2011/05/huawei-hg532c-nuevo-router-80211n-3g-de.html
* http://wiki.openwrt.org/doc/techref/bootloader/cfe
* http://wiki.openwrt.org/toh/tp-link/td-w8960n
* http://blog.hajma.cz/2011/02/customizing-asus-am200g-v-firmware.html
* https://forum.openwrt.org/viewtopic.php?id=11304
* http://developer.mips.com/tools/compilers/open-source-toolchain-linux/
* http://files.meetup.com/1590495/debugging-with-qemu.pdf
* http://www.tik.ee.ethz.ch/education/lectures/TI1/materials/assemblylanguageprogdoc.pdf
* http://w00tsec.blogspot.com.br/2013/08/simet-box-firmware-analysis-embedded.html
* http://internetcensus2012.bitbucket.org/paper.html
* http://www.devttys0.com/2011/11/exploiting-embedded-systems-part-4/
* http://www.securelist.com/en/blog/208193852/The_tale_of_one_thousand_and_one_DSL_modems
* http://www.devttys0.com/2011/09/exploiting-embedded-systems-part-1/

## Authors

* **José Ignacio Bravo** - *Initial work* - nacho.bravo@gmail.com

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* LZMA_C extract from 7zip source code (old)
