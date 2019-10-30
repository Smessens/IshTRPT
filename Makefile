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


# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic

#	gcc -lz -lm src/receiver src/packet_implem.c src/packet_implem.h src/read.c src/read.h src/receiver.c src/receiver.h src/selective.c src/selective.h

# Default target
make:
	@touch file00.dat
	@touch recu.jpg
	@rm -f src/receiver #change cleaned
	@touch src/receiver
	@rm file00.dat
	@rm recu.jpg
	gcc  -o src/receiver src/packet_implem.c  src/read.c  src/receiver.c src/selective.c -lz -lm
	./src/receiver -o "file%00d.dat" :: 64341
	@cat log.txt
	sha
#	@display recu.jpg
sha:
	sha512sum file00.dat
	sha512sum file.dat

sender:
	 ./senderprof -f file.dat ::1 64342

receiver:
	./receiverprof -o out  :: 64341


link:    # e=err -d délai -j écart -l lost
	./link_sim -p 64342 -P 64341 -e 20 -l 20  -d 30
	./senderbis -f file.dat ::1 64342

closer:
	gcc closer.c -o closer
	./closer ::1
	
test:
	clean
	@touch test_basique_out.dat test_erreur_out.dat test_delay_out.dat test_loss_out.dat test_all_out.dat
	@rm test_basique_out.dat test_erreur_out.dat test_delay_out.dat test_loss_out.dat test_all_out.dat
	gcc -o tests/test tests/test.c
	./tests/test  #add arguments

#receiver:src/receiver.c
#	gcc -o src/receiver src/receiver.c
#	./src/receiver -m 4 ::1 63345 2> log.txt
#	./src/receiver -o "fichier_%02d.dat" ::1 63345

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean chat

.PHONY: clean

clean:
	@rm -f src/receiver #change cleaned
	@touch src/receiver
