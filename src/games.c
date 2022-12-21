#include "games.h"
#include <dirent.h>

#define GAME_DIR_LEN                6 /* 'GAMES/' */
#define GAME_PREFIX_LEN             5 /* 'GAME_' */
#define TXT_EXT_LEN                 4 /* '.txt' */
#define DATE_LEN                    15 /* 'YYYYMMDD_HHMMSS_' or '_DDMMYYYY_HHMMSS' */
#define GAME_RESULT_LEN             1 /* 'W', 'F' or 'Q' */
#define ACTIVE_GAME_PATH_LEN        GAME_DIR_LEN + GAME_PREFIX_LEN + PLID_LEN + TXT_EXT_LEN
#define ARCHIVED_PLID_DIR_LEN       GAME_DIR_LEN + PLID_LEN + 1 /* '/' */
#define ARCHIVED_GAME_PATH_LEN      ARCHIVED_PLID_DIR_LEN + DATE_LEN + GAME_RESULT_LEN + TXT_EXT_LEN
#define TRIAL_GUESS_LEN             2 /* T Count */ + 1 /* '/' */ + 2 /* G Count */
#define ERROR_RATIO_LEN             1 /* T errors made*/ + 1 /* '/' */ + 1 /* G errors made*/ + 1 /* '/' */ + 1 /* max_errors */
#define GAME_FILE_HEADER_MAX        WORD_MAX + 1 /* ' ' */ + FNAME_LEN + 1 /* ' ' */ + TRIAL_GUESS_LEN + 1 /* ' ' */ + ERROR_RATIO_LEN + 1 /* '/n' */
#define FGAME_COMMAND_LEN           1
#define FGAME_LINE_LEN              FGAME_COMMAND_LEN + 1 /* ' ' */ + WORD_MAX + 1 /* '\n' */ 
#define ALPHABET_LEN                26


typedef struct word_list {
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    struct word_list *next;
} Word_Node;

const char GAMES_DIR[] = "GAMES/";
const char SCORES_DIR[] = "SCORES/";
const char WORDS_DIR[] = "src/data/words/";
const char HINTS_DIR[] = "src/data/images/";
Word_Node *word_to_choose;

/*
Searches and opens the last game from plid with mode.
If the last game is active, is_active is modified to TRUE, else FALSE.
Returns the FILE pointer to the opened file if it exists, else NULL.
*/
FILE * open_last_game(int *is_active, const char *plid, const char *mode) {
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char archived_game_dir_path[ARCHIVED_PLID_DIR_LEN + 1];
    struct dirent **filelist;
    int n_entries, found;
    FILE *f;
    snprintf(active_game_path, ACTIVE_GAME_PATH_LEN + 1, "GAMES/%s.txt", plid);
    snprintf(archived_game_dir_path, ARCHIVED_PLID_DIR_LEN + 1, "GAMES/%s/", plid);
    if ((f = fopen(active_game_path, "r"))) { // Check if file exists
        if (strcmp(mode, "r")) {
            fclose(f);
            f = fopen(active_game_path, mode);
        }
        *is_active = TRUE;
        return f;
    } else if ((n_entries = scandir(archived_game_dir_path, &filelist, 0, alphasort)) > 0) {
        for (; n_entries > 0; n_entries--) {
            if (filelist[n_entries]->d_name[0] != '.') {
                snprintf(archived_game_path, ARCHIVED_GAME_PATH_LEN + 1, "GAMES/%s/%s", plid, filelist[n_entries]->d_name);
                found = 1;
            }
            free(filelist[n_entries]);
            if (found)
                break;
        }
        free(filelist);
        f = fopen(archived_game_path, "r");
        *is_active = FALSE;
        return f;
    }
    return NULL;
}

int save_word_list(char *fname) {
    FILE *fp;
    Word_Node *new_word, *prev, *head;
    char path[15 + FNAME_LEN + 1];
    char line[WORD_MAX + 1 + FNAME_LEN + 2];
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];

    snprintf(path, 15 + FNAME_LEN + 1, "%s%s", WORDS_DIR, fname);
    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    while (fgets(line, WORD_MAX + 1 + FNAME_LEN + 2, fp)) {
        if (sscanf(line, "%s %s", word, hint) != 2)
            return -1;
        new_word = (Word_Node *)malloc(sizeof(Word_Node));
        if (prev == NULL)
            head = new_word;
        else
            prev->next = new_word;
        strcpy(new_word->word, word);
        strcpy(new_word->hint, hint);
        new_word->next = head;
        prev = new_word;
    }
    word_to_choose = head;
    return 1;
}

void free_word_list() {
    Word_Node *head = word_to_choose, *next;
    while (head != NULL)
        next = head->next;
    free(head);
    head = next;
}

Word_Node *choose_word() {
    Word_Node *chosen;
    // Repeat random number of times to make selection random.
    // !! Critical section
    chosen = word_to_choose;       // Read Critical
    word_to_choose = chosen->next; // Write Critical
    // !! Critical section over
    return chosen;
}

int count_unique_chars(char *word) {
    int alphabet[ALPHABET_LEN] = {0};
    int sum = 0;
    for (; *word != '\0'; word++)
        alphabet[*word - 'A']++;
    for (int i = 0; i<ALPHABET_LEN; i++) {
        if (alphabet[i] != 0)
            sum++;   
    }
    return sum;
}

void start_new_game(char *response, char *plid) {
    /*
     * 1. Search for active game ? return response "NOK"
     * 2. Create file in GAMES_DIR with the name GAMES_plid.txt
     * 3. Choose next word
     * 4. Write in file "word hint\n"
     * 5. Count n_letters & assess max_errors
     * 6. Return response "OK"
     */
    FILE *fp;
    int is_active = FALSE;
    int n_letters, max_errors;
    Word_Node *chosen_word;
    fp = open_last_game(&is_active, plid, "w");
    if (fp == NULL)
        return;
    if (is_active) {
        strcpy(response, "RSG NOK\n");
        return;
    }
    chosen_word = choose_word();
    n_letters = strlen(chosen_word->word);
    if (n_letters >= 11)
        max_errors = 9;
    else if (n_letters >= 7)
        max_errors = 8;
    else
        max_errors = 7;
    fprintf(fp, "%s %s %d %d/%d %d/%d/%d\n", 
        chosen_word->word, chosen_word->hint, 
        count_unique_chars(chosen_word->word),
        0 /*T count*/, 0 /*G count*/, 
        0 /*T errors*/, 0 /*G errors*/,  max_errors);
    
    fflush(fp);
    fclose(fp);
    snprintf(response, CMD_ID_LEN + 1 + 2 + 1 + 2 + 1 + 2 + 2, "RSG OK %d %d\n", n_letters, max_errors);
    return;
}

int was_already_guessed(FILE *fp, char c, char *word) {
    long cur = ftell(fp);
    char line[FGAME_LINE_LEN + 1];
    char code;
    char play[WORD_MAX + 1];
    while (fgets(line, FGAME_LINE_LEN + 1, fp)) {
        if (sscanf(line, "%c %s\n", &code, play) != 2) 
            continue;
        if (code == 'T') {
            if (c == *play) {
                fseek(fp, -cur, SEEK_CUR);
                return TRUE;
            }   
        } else if (code == 'G') {
            if (word != NULL && !strcmp(play, word)) {
                fseek(fp, -cur, SEEK_CUR);
                return TRUE;
            }
        }
    }
    fseek(fp, -cur, SEEK_CUR);
    return FALSE;
}

int get_char_match_word(char *word, char c, char *pos) {
    int t = 0;
    for (int i = 1; *word != '\0'; i++, word++) {
        if (*word != c)
            continue;
        if (i > 10)
            *(pos++) = i / 10 + '0';
        *(pos++) = i % 10 + '0';
        *(pos++) = ' ';
        t++;
    }
    if (t > 0)
        pos--;
    *(pos) = '\0';
    return t;
}

void play_letter_guess(char *response, char *plid, char c, int trial) {
    /*
     * 2. Search for active game ? return response "ERR"
     * 3. Get word from active game.
     * 4. Count how many trials have been done / errors
     * 5. c has been guessed before ? return "DUP"
     * 6. Match trial client and server ? return "INV"
     * 7. Search for c in word
     * 8. Is c in word ? return "WIN" or "OK + n + pos*"
     * 9. Else ? return "OVR" or "NOK"
     */
    FILE *fp;
    int is_active = FALSE;
    char header[GAME_FILE_HEADER_MAX + 1];
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    char pos[WORD_MAX*2];
    int s_trial, t_count, g_count;
    int unique_chars;
    int t_errors, g_errors, max_errors;
    int n;

    fp = open_last_game(&is_active, plid, "r+");
    if (fp == NULL)
        return;
    if (!is_active) {
        strcpy(response, "RLG ERR\n");
        return;
    }
    if (trial <= 0) {
        strcpy(response, "RLG ERR\n");
        return;
    }
    fgets(header, GAME_FILE_HEADER_MAX + 1, fp);
    sscanf(header, "%s %s %d %d/%d %d/%d/%d\n", 
        word, hint, &unique_chars, 
        &t_count, &g_count, 
        &t_errors, &g_errors, &max_errors);
    
    s_trial = t_count + g_count;
    if (was_already_guessed(fp, c, NULL)) {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG DUP %d\n", s_trial);
        return;
    }
    if (trial != s_trial + 1) {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG INV %d\n", s_trial);
        return;
    }
    n = get_char_match_word(word, c, pos);
    t_count++;
    s_trial++;
    if (n == 0) {
        t_errors++;
        if (t_errors + g_errors == max_errors)
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG OVR %d\n", s_trial);
        else
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG NOK %d\n", s_trial);
    } else if (t_count - t_errors == unique_chars) {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG WIN %d\n", s_trial);
    } else {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 1 + 2 + 1 + strlen(pos) + 2, "RLG OK %d %d %s\n", s_trial, n, pos);
    }
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%s %s %d %d/%d %d/%d/%d\n",
        word, hint, unique_chars,
        t_count, g_count,
        t_errors, g_errors, max_errors);
    fseek(fp, 0, SEEK_END);
    fprintf(fp, "T %c\n", c);
    fflush(fp);
    fclose(fp);
    return;
}

void play_word_guess(char *response, char *plid, char *guess, int trial) {
    /*
     * 2. Search for active game ? return "ERR"
     * 3. Get word from active game.
     * 4. Count how many trials have been done / errors
     * 5. word has been guessed before ? return "DUP"
     * 6. Match trial client and server ? return "INV"
     * 7. Match client_word with server_word
     * 8. Is the same? return "WIN"
     * 9. Else ? return "OVR" or "NOK"
     */
    FILE *fp;
    int is_active = FALSE;
    char header[GAME_FILE_HEADER_MAX + 1];
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    int s_trial, t_count, g_count;
    int unique_chars;
    int t_errors, g_errors, max_errors;
    int n;

    fp = open_last_game(&is_active, plid, "r+");
    if (fp == NULL)
        return;
    if (!is_active) {
        strcpy(response, "RWG ERR\n");
        return;
    }
    fgets(header, GAME_FILE_HEADER_MAX + 1, fp);
    sscanf(header, "%s %s %d %d/%d %d/%d/%d\n", 
        word, hint, &unique_chars, 
        &t_count, &g_count, 
        &t_errors, &g_errors, &max_errors);
    
    s_trial = t_count + g_count;
    if (was_already_guessed(fp, '\0', guess)) {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG DUP %d\n", s_trial);
        return;
    }
    if (trial != s_trial + 1) {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG INV %d\n", s_trial);
        return;
    }
    n = strcmp(word, guess);
    t_count++;
    s_trial++;
    if (n != 0) {
        g_errors++;
        if (t_errors + g_errors == max_errors)
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG OVR %d\n", s_trial);
        else
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG NOK %d\n", s_trial);
    } else {
        snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG WIN %d\n", s_trial);
    }
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%s %s %d %d/%d %d/%d/%d\n",
        word, hint, unique_chars,
        t_count, g_count,
        t_errors, g_errors, max_errors);
    fseek(fp, 0, SEEK_END);
    fprintf(fp, "G %s\n", guess);
    fflush(fp);
    fclose(fp);
    return;
}

void quit(char *response, char *plid) {
    /*
     * 2. Search for active game ? return "NOK"
     * 3. Archive active game
     * 4. return "OK"
     */
}

void rev(char *response, char *plid) {
    /*
     * 0. Invalid sintax ? return "ERR"
     * 1. Invalid plid ? return "ERR"
     * 2. Search for active game ? no return
     * 3. return "word"
     */
}

void get_scoreboard(char *response, pid_t pid) {
    /*
     * 1. Find top 10 scores in SCORES/
     * 2. Is empty ? return status = EMPTY
     * 3. Build Fname = TOPSCORES_nnnnnnn.txt, with nnnnnnn being the pid
     * 4. Parse to pretended format and count chars
     * 5. Build Fsize = chars counted
     * 6. Send to socket the data + '\n'
     */
}

void get_hint_image(char *response, char *plid) {
    /*
     * 2. Search for active game ? return "NOK"
     * 3. Get hint fname
     * 4. Get file size
     * 5. Send directly from file to socket + '\n'
     */
}

void get_state(char *response, char *plid) {
    /*
     * 2. Search for active game or last recently played
     * 3. Get game file name
     * 4. Get game file size
     * 5. Send directly from file to socket + '\n'
     */
}