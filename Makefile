CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc
LDFLAGS = -lm

SRC = platform/pc/main.c src/hal/sensor_hal_sim.c src/core/window.c src/core/stats.c src/core/detector.c src/app/fsm.c src/output/output_hal_sim.c
OUT = sentinela.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)