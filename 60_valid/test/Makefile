CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 \
          -I ../../30_src/components/midlevel/include \
          -I ../../30_src/components/app/include

SRCS    = test_main.c \
          ../../30_src/components/midlevel/drv_ina237_convert.c \
          ../../30_src/components/app/app_power_monitor_logic.c

BIN     = test_runner

.PHONY: all test clean

all: test

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(BIN)

test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
