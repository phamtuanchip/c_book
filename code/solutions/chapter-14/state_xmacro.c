// state_xmacro.c
#include <stdio.h>
#include <string.h>

#define STATE_LIST(X) X(IDLE) X(RUNNING) X(PAUSED) X(STOPPED)

typedef enum {
#define X(name) STATE_##name,
    STATE_LIST(X)
#undef X
    STATE_COUNT
} State;

static const char *const STATE_NAMES[] = {
#define X(name) [STATE_##name] = #name,
    STATE_LIST(X)
#undef X
};

static const char *state_name(State s) { return (s >= 0 && s < STATE_COUNT) ? STATE_NAMES[s] : "?"; }

/* Trả STATE_COUNT nếu không nhận ra */
static State state_from_string(const char *s) {
    for (int i = 0; i < STATE_COUNT; i++)
        if (strcmp(STATE_NAMES[i], s) == 0) return (State)i;
    return STATE_COUNT;
}

int main(void) {
    printf("%s %d\n", state_name(STATE_PAUSED), (int)state_from_string("RUNNING"));   // PAUSED 1
    return 0;
}
