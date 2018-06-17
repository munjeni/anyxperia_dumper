CFLAGS= -Wall -O3 -Iinclude -Izlib-1.2.9 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1

CC=gcc
STRIP=strip

CCWIN=i586-mingw32msvc-gcc
CCWINSTRIP=i586-mingw32msvc-strip

CCARM=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc
CCARMSTRIP=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip

CCARM64=/home/savan/Desktop/qemu/busybox-1.23.2/busybox-1.23.2/busybox-w32/buildroot-2017.02.2/output/host/usr/bin/aarch64-buildroot-linux-uclibc-gcc
CCARM64STRIP=/home/savan/Desktop/qemu/busybox-1.23.2/busybox-1.23.2/busybox-w32/buildroot-2017.02.2/output/host/usr/bin/aarch64-buildroot-linux-uclibc-strip

CCAPPLE64=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/x86_64-apple-darwin11-cc
CCAPPLESTRIP64=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/x86_64-apple-darwin11-strip

CCAPPLE=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/i386-apple-darwin11-cc
CCAPPLESTRIP=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/i386-apple-darwin11-strip

CCMIPS64=/home/savan/Desktop/buildroot-2017.02.8/output/host/usr/bin/mips64-buildroot-linux-uclibc-gcc
CCMIPS64STRIP=/home/savan/Desktop/buildroot-2017.02.8/output/host/usr/bin/mips64-buildroot-linux-uclibc-strip

CCMIPS=/home/savan/Desktop/buildroot-2018.02.2/output/host/usr/bin/mips-buildroot-linux-uclibc-gcc
CCMIPSSTRIP=/home/savan/Desktop/buildroot-2018.02.2/output/host/usr/bin/mips-buildroot-linux-uclibc-strip

SOURCE=   \
     zlib-1.2.9/adler32.c \
     zlib-1.2.9/crc32.c \
     zlib-1.2.9/deflate.c \
     zlib-1.2.9/infback.c \
     zlib-1.2.9/inffast.c \
     zlib-1.2.9/inflate.c \
     zlib-1.2.9/inftrees.c \
     zlib-1.2.9/trees.c \
     zlib-1.2.9/zutil.c \
     zlib-1.2.9/compress.c \
     zlib-1.2.9/uncompr.c \
     zlib-1.2.9/gzclose.c \
     zlib-1.2.9/gzlib.c \
     zlib-1.2.9/gzread.c \
     zlib-1.2.9/gzwrite.c \
     lz4.c \
     unpackbootimg.c \
     untar.c \
     kernel_dump.c

default:kernel_dump.exe kernel_dump.i386 kernel_dump.x86_64 kernel_dump.arm32 kernel_dump.arm64 kernel_dump.x86_64-apple-darwin11 kernel_dump.i386-apple-darwin11 kernel_dump.mips64 kernel_dump.mips32

kernel_dump.exe:
	${CCWIN} ${CFLAGS} -static ${SOURCE} -o kernel_dump.exe
	${CCWINSTRIP} kernel_dump.exe

kernel_dump.i386:
	${CC} -m32 ${CFLAGS} -static ${SOURCE} -o kernel_dump.i386
	${STRIP} kernel_dump.i386

kernel_dump.x86_64:
	${CC} ${CFLAGS} -static ${SOURCE} -o kernel_dump.x86_64
	${STRIP} kernel_dump.x86_64

kernel_dump.arm32:
	${CCARM} ${CFLAGS} -static ${SOURCE} -o kernel_dump.arm32
	${CCARMSTRIP} kernel_dump.arm32

kernel_dump.arm64:
	${CCARM64} ${CFLAGS} -static ${SOURCE} -o kernel_dump.arm64
	${CCARM64STRIP} kernel_dump.arm64

kernel_dump.i386-apple-darwin11:
	${CCAPPLE} ${CFLAGS} ${SOURCE} -o kernel_dump.i386-apple-darwin11
	${CCAPPLESTRIP} kernel_dump.i386-apple-darwin11

kernel_dump.x86_64-apple-darwin11:
	${CCAPPLE64} ${CFLAGS} ${SOURCE} -o kernel_dump.x86_64-apple-darwin11
	${CCAPPLESTRIP64} kernel_dump.x86_64-apple-darwin11

kernel_dump.mips64:
	${CCMIPS64} ${CFLAGS} -static ${SOURCE} -o kernel_dump.mips64
	${CCMIPS64STRIP} kernel_dump.mips64

kernel_dump.mips32:
	${CCMIPS} ${CFLAGS} -static ${SOURCE} -o kernel_dump.mips32
	${CCMIPSSTRIP} kernel_dump.mips32

clean:
	rm -f kernel_dump.exe kernel_dump.i386 kernel_dump.x86_64 kernel_dump.arm32 kernel_dump.arm64 kernel_dump.x86_64-apple-darwin11 kernel_dump.i386-apple-darwin11 kernel_dump.mips64 kernel_dump.mips32

distclean:
	rm -f kernel_dump.exe kernel_dump.i386 kernel_dump.x86_64 kernel_dump.arm32 kernel_dump.arm64 kernel_dump.x86_64-apple-darwin11 kernel_dump.i386-apple-darwin11 kernel_dump.mips64 kernel_dump.mips32

