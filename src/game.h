#include "common.h"

void init_game(char *plid);
char * get_plid();
char * get_word();
int get_word_len();
int get_no_tries();
int get_max_errors();
char get_last_letter();
void set_last_letter(char c);
void start_game(char *output, int n_letters, int max_errrors);
void play_game(char *output, int n, int pos[]);
void guess_game(char *output, char *word);
void quit_game();