# makefile configuration

COMMON_OBJECTS =

CPU             = msp430g2553

CFLAGS          = -mmcu=${CPU} -I./timerLib -L/opt/ti/msp430_gcc/include -Os



# switch the compiler (for the internal make rules)

CC              = msp430-elf-gcc

AS              = msp430-elf-as



all: toy.elf



# link everything into the final ELF

toy.elf: ${COMMON_OBJECTS} main.o buzzer.o timerLib/clocksTimer.o timerLib/sr.o

	${CC} ${CFLAGS} -o $@ $^



load: toy.elf

	msp430loader.sh $<



clean:

	rm -f *.o toy.elf timerLib/*.o



# simple deps (optional; GCC will also infer)

buzzer.c: buzzer.h

