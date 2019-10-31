# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo
CFLAGS += -lz
CFLAGS += -lm

CUNIT   = $(HOME)/local

CFLAGS  = -I$(CUNIT)/include
# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic
LDFLAGS = -L$(CUNIT)/lib
LIBS    = -lcunit

# Default target
make:
	@touch file00.dat
	@touch recu.jpg
	@rm -f src/receiver #change cleaned
	@touch src/receiver
	@rm file00.dat
	@rm recu.jpg
	gcc  -o receiver src/packet_implem.c  src/read.c  src/receiver.c src/selective.c -lz -lm -g

#	@display
sha:
	sha512sum file00.dat
	sha512sum file.dat

valg:
	gcc  -o receiver src/packet_implem.c  src/read.c  src/receiver.c src/selective.c -lz -lm -g
	valgrind --log-file=log.txt ./receiver -o "file%00d.dat" :: 64341 #--leak-check=yes --track-origins=yes --show-leak-kinds=all ./receiver -o "file%00d.dat" :: 64341

sender:
	./senderprof -f file.dat ::1  64342

receiver:
	./receiverprof -o out 2001:6a8:308f:5:f68e:38ff:fe74:5f0a  64341


link:    # e=err -d délai -j écart -l lost
	./link_sim -p 64342 -P 64341 -e 10 -l 10

test: tests/tests.c
			cd tests && make
			cd tests && ./tests
			@rm -f tests/tests.o



# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean chat

.PHONY: clean

clean:
	@rm  src/receiver #change cleaned
	@touch src/receiver
