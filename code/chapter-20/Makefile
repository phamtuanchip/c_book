CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Wshadow -O2 -g -D_FORTIFY_SOURCE=2 -fstack-protector-strong
LDLIBS  := -pthread

server: server.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

asan: server.c
	$(CC) -std=c11 -Wall -Wextra -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer $< -o server_asan $(LDLIBS)

tsan: server.c
	$(CC) -std=c11 -Wall -Wextra -g -O1 -fsanitize=thread $< -o server_tsan $(LDLIBS)

clean:
	rm -f server server_asan server_tsan
