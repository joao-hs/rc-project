#include "common.h"

typedef struct state {
    char plid[PLID_LEN];
    char word[WORD_MAX];
    int tries;
    int errors;
} State;