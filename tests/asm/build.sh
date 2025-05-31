make clean
mv code/$1.asm .
make FILE=$1.asm
mv $1.asm code
rm $1.o
mv $1.elf bin