 /usr/local/Cellar/llvm/5.0.1/bin/scan-build /usr/bin/make -e TARGET="pc_dbg_kern_" -e CFLAGS="-g -Wall -Werror -arch i386 -I /Users/andrewthomson/Projects/kernelpanic" -e ARCH_MACRO=PC_BUILD -e DEBUG_MACRO=DEBUG -e BUILD_MACRO=KERNEL_BUILD -e OS_MACRO="DARWIN" -f makefile.main umouse

Note, I had to remove the cc var, so that the injection of scan-build would work.

/usr/local/Cellar/llvm/5.0.1/bin/scan-view /var/folders/1q/l4nn1wtn1k77vmdg7lbnyd8h0000gn/T/scan-build-2018-02-17-113347-81034-1

