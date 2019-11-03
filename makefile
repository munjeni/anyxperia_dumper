CFLAGS= -Wall -O2 -Iinclude -Izlib-1.2.11 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1

CC=gcc
STRIP=strip

CCWIN=i686-w64-mingw32-gcc
CCWINSTRIP=i686-w64-mingw32-strip

CCARM=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc
CCARMSTRIP=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip

CCARM64=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
CCARM64STRIP=/home/savan/Desktop/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-strip

CCAPPLE64=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/x86_64-apple-darwin11-cc
CCAPPLESTRIP64=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/x86_64-apple-darwin11-strip

CCAPPLE=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/i386-apple-darwin11-cc
CCAPPLESTRIP=/home/savan/Desktop/osxtoolchain/osxcross/target/bin/i386-apple-darwin11-strip

CCMIPS64=/home/savan/Desktop/buildroot-2017.02.8/output/host/usr/bin/mips64-buildroot-linux-uclibc-gcc
CCMIPS64STRIP=/home/savan/Desktop/buildroot-2017.02.8/output/host/usr/bin/mips64-buildroot-linux-uclibc-strip

CCMIPS=/home/savan/Desktop/buildroot-2018.02.2/output/host/usr/bin/mips-buildroot-linux-uclibc-gcc
CCMIPSSTRIP=/home/savan/Desktop/buildroot-2018.02.2/output/host/usr/bin/mips-buildroot-linux-uclibc-strip

SOURCE=   \
     zlib-1.2.11/adler32.c \
     zlib-1.2.11/crc32.c \
     zlib-1.2.11/deflate.c \
     zlib-1.2.11/infback.c \
     zlib-1.2.11/inffast.c \
     zlib-1.2.11/inflate.c \
     zlib-1.2.11/inftrees.c \
     zlib-1.2.11/trees.c \
     zlib-1.2.11/zutil.c \
     zlib-1.2.11/compress.c \
     zlib-1.2.11/uncompr.c \
     zlib-1.2.11/gzclose.c \
     zlib-1.2.11/gzlib.c \
     zlib-1.2.11/gzread.c \
     zlib-1.2.11/gzwrite.c \
     lz4.c \
     unpackbootimg.c \
     untar.c \
     sony_dump.c

default:download sony_dump.exe sony_dump.i386 sony_dump.x86_64 sony_dump.arm32 sony_dump.arm32_pie sony_dump.arm64 sony_dump.arm64_pie sony_dump.x86_64-apple-darwin11 sony_dump.i386-apple-darwin11 sony_dump.mips64 sony_dump.mips32 archive

download:
	@if [ ! -d "zlib-1.2.11" ]; then wget https://zlib.net/zlib-1.2.11.tar.gz ; tar xzf zlib-1.2.11.tar.gz ; rm -rf zlib-1.2.11.tar.gz ; fi

sony_dump.exe:
	${CCWIN} ${CFLAGS} -static ${SOURCE} -o sony_dump.exe
	${CCWINSTRIP} sony_dump.exe

sony_dump.i386:
	${CC} -m32 ${CFLAGS} -static ${SOURCE} -o sony_dump.i386
	${STRIP} sony_dump.i386

sony_dump.x86_64:
	${CC} ${CFLAGS} -static ${SOURCE} -o sony_dump.x86_64
	${STRIP} sony_dump.x86_64

sony_dump.arm32:
	${CCARM} ${CFLAGS} -static ${SOURCE} -o sony_dump.arm32
	${CCARMSTRIP} sony_dump.arm32

sony_dump.arm32_pie:
	@cp -fr sony_dump.arm32 sony_dump.arm32_pie
	@dd if=pie of=sony_dump.arm32_pie bs=1 count=1 seek=16 conv=notrunc

sony_dump.arm64:
	${CCARM64} ${CFLAGS} -static ${SOURCE} -o sony_dump.arm64
	${CCARM64STRIP} sony_dump.arm64

sony_dump.arm64_pie:
	@cp -fr sony_dump.arm64 sony_dump.arm64_pie
	@dd if=pie of=sony_dump.arm64_pie bs=1 count=1 seek=16 conv=notrunc

sony_dump.i386-apple-darwin11:
	${CCAPPLE} ${CFLAGS} ${SOURCE} -o sony_dump.i386-apple-darwin11
	${CCAPPLESTRIP} sony_dump.i386-apple-darwin11

sony_dump.x86_64-apple-darwin11:
	${CCAPPLE64} ${CFLAGS} ${SOURCE} -o sony_dump.x86_64-apple-darwin11
	${CCAPPLESTRIP64} sony_dump.x86_64-apple-darwin11

sony_dump.mips64:
	${CCMIPS64} ${CFLAGS} -static ${SOURCE} -o sony_dump.mips64
	${CCMIPS64STRIP} sony_dump.mips64

sony_dump.mips32:
	${CCMIPS} ${CFLAGS} -static ${SOURCE} -o sony_dump.mips32
	${CCMIPSSTRIP} sony_dump.mips32

archive:
	@zip -9 sony_dump_tool.zip sony_dump.arm32_pie sony_dump.arm64_pie sony_dump.exe sony_dump.i386-apple-darwin11 sony_dump.mips64 sony_dump.x86_64-apple-darwin11 sony_dump.arm32 sony_dump.arm64 sony_dump.i386 sony_dump.mips32 sony_dump.x86_64

clean:
	rm -rf sony_dump_tool.zip sony_dump.exe sony_dump.i386 sony_dump.x86_64 sony_dump.arm32 sony_dump.arm32_pie sony_dump.arm64 sony_dump.arm64_pie sony_dump.x86_64-apple-darwin11 sony_dump.i386-apple-darwin11 sony_dump.mips64 sony_dump.mips32

distclean:
	rm -rf sony_dump_tool.zip zlib-1.2.11 sony_dump.exe sony_dump.i386 sony_dump.x86_64 sony_dump.arm32 sony_dump.arm32_pie sony_dump.arm64 sony_dump.arm64_pie sony_dump.x86_64-apple-darwin11 sony_dump.i386-apple-darwin11 sony_dump.mips64 sony_dump.mips32

