#include "game.h"
#include "socket.h"

typedef struct state {
    char *plid;
    char *word;
    int word_len;
    int no_tries;
    int max_errors;
    char last_letter;
} State;


State game_state;

void init_game(char *plid) {
    size_t offset = PLID_LEN - strlen(plid);
    game_state.plid = (char *)malloc((PLID_LEN + 1) * sizeof(char));
    if (game_state.plid != NULL)
        strcpy(game_state.plid + offset, plid);
    game_state.word = NULL;
    game_state.word_len = -1;
    game_state.no_tries = -1;
    game_state.max_errors = -1;
    game_state.last_letter = '\0';
}

void free_game() {
    free(game_state.plid);
    free(game_state.word);
}

/* GETTERS */

char * get_plid() {
    return game_state.plid;
}

char * get_word() {
    return game_state.word;
}

int get_word_len() {
    return game_state.word_len;
}

int get_no_tries() {
    return game_state.no_tries;
}

int get_max_errors() {
    return game_state.max_errors;
}

char get_last_letter() {
    return game_state.last_letter;
}

/* SETTERS */

void set_last_letter(char c) {
    game_state.last_letter = c;
}

/* VERIFIERS */

int is_game_valid() {
    if (game_state.no_tries <= 0 || game_state.max_errors <= 0 || game_state.plid == NULL)
        return 0;
    return 1;
}

int is_word_complete() {
    for (int i = 0; i < game_state.word_len; i += 2) {
        if (game_state.word[i] == '_')
            return 0;
    }
    return 1;
}

/* MODIFIERS */

/*
Decreases max_errors, increases no_tries.
Returns:
 * -1, if game is not valid;
 * 0, if game ended;
 * no_tries, if game is active.
*/
int wrong_try() {
    if (!is_game_valid())
        return -1;
    game_state.no_tries += 1;
    game_state.max_errors -= 1;
    if (game_state.max_errors == 0)
        return 0;
    return game_state.no_tries;
}

int right_try() {
    if (!is_game_valid())
        return -1;
    game_state.no_tries += 1;
    return game_state.no_tries;
}

/* COMMAND METHODS */

void start_game(char *output, int n_letters, int max_errors) {
    int i;
    game_state.word = (char *)malloc((n_letters*2) * sizeof(char));
    for (i = 0; i < (n_letters*2); i+=2) {
        game_state.word[i] = '_';
        game_state.word[i+1] = ' ';
    }
    game_state.no_tries = 0;
    game_state.max_errors = max_errors;
    sprintf(output, "New game started (max %d errors): %s\n", game_state.max_errors, game_state.word);
}

void play_game(char *output, int n, int pos[]) {
    int index;
    char c = get_last_letter();
    for (int i = 0; i < n; i++) {
        index = pos[i]*2;
        game_state.word[index] = c;
    }
    right_try();
    sprintf(output, "Word: %s\n", game_state.word);
}

void guess_game(char *output, char *word) {
    for (int i = 0; i < game_state.word_len; i += 2) {
        if (game_state.word[i] == '_')
            game_state.word[i] = word[i/2];
    }
    right_try();
    sprintf(output, "WELL DONE! You guessed %s\n", game_state.word);
}

void quit_game() {
    free_game();
}