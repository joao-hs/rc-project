#include "common.h"

int save_word_list(char *fname);
void free_word_list();
void start_new_game(char *response, char *plid);
void play_letter_guess(char *response, char *plid, char c, int trial);
void play_word_guess(char *response, char *plid, char *guess, int trial);
void quit(char *response, char *plid);
int rev(char *response, char *plid);
int get_scoreboard(char *response, pid_t pid, FILE **fp);
int get_hint_image(char *response, char *plid, FILE **fp);
int get_state(char *response, char *plid, FILE **fp);