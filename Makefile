# Makefile
# Builds the PC simulation binary using gcc (MSYS2 UCRT64).

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc

SRC = platform/pc/main.c src/hal/sensor_hal_sim.c
OUT = sentinela.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)