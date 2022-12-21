#include "common.h"

void start_new_game(char *response, char *plid);
void play_letter_guess(char *response, char *plid, char c, int trial);
void play_word_guess(char *response, char *plid, char *guess, int trial);
void quit(char *response, char *plid);
void rev(char *response, char *plid);
void get_scoreboard(char *response, pid_t pid);
void get_hint_image(char *response, char *plid);
void get_state(char *response, char *plid);