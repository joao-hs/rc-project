#include "games.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define GAME_DIR_LEN 6    /* 'GAMES/' */
#define SCORE_DIR_LEN 7   /* 'SCORES/' */
#define TMP_DIR_LEN 5     /* '.tmp/' */
#define GAME_PREFIX_LEN 5 /* 'GAME_' */
#define SCORE_LEN 3       /* '001' */
#define DATE_FORMAT 15
#define SCORE_PREFIX_LEN SCORE_LEN + 1 /* '100_' */
#define TXT_EXT_LEN 4                  /* '.txt' */
#define DATE_LEN 15                    /* 'YYYYMMDD_HHMMSS_' or '_DDMMYYYY_HHMMSS' */
#define GAME_RESULT_LEN 1              /* 'W', 'F' or 'Q' */
#define ACTIVE_GAME_PATH_LEN GAME_DIR_LEN + GAME_PREFIX_LEN + PLID_LEN + TXT_EXT_LEN
#define ARCHIVED_PLID_DIR_LEN GAME_DIR_LEN + PLID_LEN + 1 /* '/' */
#define ARCHIVED_GAME_PATH_LEN ARCHIVED_PLID_DIR_LEN + DATE_LEN + 1 /* '_' */ + GAME_RESULT_LEN + TXT_EXT_LEN
#define SCORE_FILE_PATH_LEN SCORE_DIR_LEN + SCORE_PREFIX_LEN + PLID_LEN + DATE_LEN + TXT_EXT_LEN
#define TRIAL_GUESS_LEN 2 /* T Count */ + 1 /* '/' */ + 2                                                                           /* G Count */
#define ERROR_RATIO_LEN 1 /* T errors made*/ + 1 /* '/' */ + 1 /* G errors made*/ + 1 /* '/' */ + 1                                 /* max_errors */
#define GAME_FILE_HEADER_MAX WORD_MAX + 1 /* ' ' */ + FNAME_LEN + 1 /* ' ' */ + TRIAL_GUESS_LEN + 1 /* ' ' */ + ERROR_RATIO_LEN + 1 /* '/n' */
#define FGAME_COMMAND_LEN 1
#define FGAME_LINE_LEN FGAME_COMMAND_LEN + 1 /* ' ' */ + WORD_MAX + 1 /* '\n' */
#define ALPHABET_LEN 26
#define SCOREBOARD_LINE_LEN SCORE_LEN + 1 /* ' ' */ + PLID_LEN + 1 /* ' ' */ + WORD_MAX + 1 /* ' ' */ + 2 /*two digits*/ + 1 /* ' ' */ + 2 /*two digits*/ + 1 /* '\n' */

typedef struct word_list {
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    struct word_list *next;
} Word_Node;

typedef struct score_info {
    int score;
    char plid[PLID_LEN + 1];
    char word[WORD_MAX + 1];
    int right_guesses;
    int total;
} Score;

const char GAMES_DIR[] = "GAMES/";
const char SCORES_DIR[] = "SCORES/";
const char WORDS_DIR[] = "src/data/words/";
const char HINTS_DIR[] = "src/data/images/";
Word_Node *word_to_choose;
int randomize = FALSE;
extern int errno;

/*
Searches for the last game from plid.
If the last game is active, returns 1.
If the last game is archived, return 0.
If there's no game record, returns -1.
The corresponding path argument is completed.
*/
int find_last_game(const char *plid, char *active_game_path, char *archived_game_path) {
    char archived_game_dir_path[ARCHIVED_PLID_DIR_LEN + 1];
    struct dirent **filelist;
    int n_entries;
    snprintf(active_game_path, ACTIVE_GAME_PATH_LEN + 1, "GAMES/%s.txt", plid);
    snprintf(archived_game_dir_path, ARCHIVED_PLID_DIR_LEN + 1, "GAMES/%s/", plid);
    FILE *check = fopen(active_game_path, "r");

    if (check) { // Check if file exists
        fclose(check);
        return 1;
    } else if ((n_entries = scandir(archived_game_dir_path, &filelist, 0, alphasort)) > 0) {
        n_entries--;
        for (; n_entries > 0; n_entries--) {
            if (filelist[n_entries]->d_name[0] == '.') {
                free(filelist[n_entries]);
                continue;
            }
            snprintf(archived_game_path, ARCHIVED_GAME_PATH_LEN + 1, "GAMES/%s/%s", plid, filelist[n_entries]->d_name);
            free(filelist[n_entries]);
            break;
        }
        free(filelist);
        return 0;
    }
    return -1;
}

int archive_game(char *plid, char *state) {
    DIR *d;
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_path[ARCHIVED_PLID_DIR_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char date[DATE_FORMAT + 1];
    int r = -1;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(date, DATE_FORMAT + 1, "%d%02d%02d_%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    snprintf(active_game_path, ACTIVE_GAME_PATH_LEN + 1, "GAMES/%s.txt", plid);
    snprintf(archived_path, ARCHIVED_PLID_DIR_LEN + 1, "GAMES/%s", plid);
    snprintf(archived_game_path, ARCHIVED_GAME_PATH_LEN + 1, "%s/%s_%s.txt", archived_path, date, state);
    d = opendir(archived_path);
    if (d) {
        closedir(d);
    } else if (ENOENT == errno) {
        if (mkdir(archived_path, 0777)) {
            return -1;
        }
    } else {
        closedir(d);
        return -1;
    }
    r = rename(active_game_path, archived_game_path);
    return r;
}

int save_score(FILE *fp, char *plid) {
    Score s;
    FILE *score;
    char hint[FNAME_LEN + 1];
    char header[GAME_FILE_HEADER_MAX + 1];
    char score_path[SCORE_FILE_PATH_LEN + 1];
    char date[DATE_FORMAT + 1];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int unique_chars, t_count, g_count, t_errors, g_errors, max_errors;

    snprintf(date, DATE_FORMAT + 1, "%02d%02d%d_%02d%02d%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fgets(header, GAME_FILE_HEADER_MAX + 1, fp);
    sscanf(header, "%s %s %d %d/%d %d/%d/%d\n",
           s.word, hint, &unique_chars,
           &t_count, &g_count,
           &t_errors, &g_errors, &max_errors);
    strcpy(s.plid, plid);
    s.right_guesses = t_count - t_errors;
    s.total = t_count + g_count;
    s.score = (s.right_guesses / s.total) * 100;
    snprintf(score_path, SCORE_FILE_PATH_LEN + 1, "SCORES/%03d_%s_%s.txt", s.score, plid, date);
    score = fopen(score_path, "w");
    if (!score) {
        return -1;
    }
    fprintf(score, "%d %s %s %d %d", s.score, s.plid, s.word, s.right_guesses, s.total);
    fclose(score);
    return 0;
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
    for (; *word != '\0'; word++) {
        if (*word >= 'A' && *word <= 'Z')
            alphabet[*word - 'A']++;
        else if (*word >= 'a' && *word <= 'z')
            alphabet[*word - 'a']++;
    }
    for (int i = 0; i < ALPHABET_LEN; i++) {
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
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    int n_letters, max_errors, code;
    Word_Node *chosen_word;

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code == 1) {
        strcpy(response, "RSG NOK\n");
        return;
    }
    fp = fopen(active_game_path, "w");
    chosen_word = choose_word();
    n_letters = strlen(chosen_word->word);
    if (n_letters >= 11)
        max_errors = 9;
    else if (n_letters >= 7)
        max_errors = 8;
    else
        max_errors = 7;
    code = count_unique_chars(chosen_word->word);
    fprintf(fp, "%s %s %d %d/%d %d/%d/%d\n",
            chosen_word->word, chosen_word->hint,
            code,
            0 /*T count*/, 0 /*G count*/,
            0 /*T errors*/, 0 /*G errors*/, max_errors);
    snprintf(response, CMD_ID_LEN + 1 + 2 + 1 + 2 + 1 + 2 + 2, "RSG OK %d %d\n", n_letters, max_errors);
    fflush(fp);
    fclose(fp);
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
        if (i >= 10)
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
    char header[GAME_FILE_HEADER_MAX + 1];
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    char pos[WORD_MAX * 2];
    int s_trial, t_count, g_count;
    int unique_chars;
    int t_errors, g_errors, max_errors;
    int n, code, win = FALSE, lose = FALSE;

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code != 1) {
        strcpy(response, "RLG ERR\n");
        return;
    }
    if (trial <= 0) {
        strcpy(response, "RLG ERR\n");
        return;
    }
    fp = fopen(active_game_path, "r+");
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
        if (t_errors + g_errors == max_errors) {
            lose = TRUE;
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG OVR %d\n", s_trial);
        } else
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RLG NOK %d\n", s_trial);
    } else if (t_count - t_errors == unique_chars) {
        win = TRUE;
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
    fseek(fp, 0, SEEK_SET);
    if (win) {
        save_score(fp, plid);
        fclose(fp);
        archive_game(plid, "W");
    }
    if (lose) {
        fclose(fp);
        archive_game(plid, "F");
    }
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
    char header[GAME_FILE_HEADER_MAX + 1];
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char word[WORD_MAX + 1];
    char hint[FNAME_LEN + 1];
    int s_trial, t_count, g_count;
    int unique_chars;
    int t_errors, g_errors, max_errors;
    int n, code, win = FALSE, lose = FALSE;

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code != 1) {
        strcpy(response, "RWG ERR\n");
        return;
    }
    if (trial <= 0) {
        strcpy(response, "RWG ERR\n");
        return;
    }
    fp = fopen(active_game_path, "r+");
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
    g_count++;
    s_trial++;
    if (n != 0) {
        g_errors++;
        if (t_errors + g_errors == max_errors) {
            lose = TRUE;
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG OVR %d\n", s_trial);
        } else
            snprintf(response, CMD_ID_LEN + 1 + 3 + 1 + 2 + 2, "RWG NOK %d\n", s_trial);
    } else {
        win = TRUE;
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
    if (win) {
        save_score(fp, plid);
        fclose(fp);
        archive_game(plid, "W");
    }
    if (lose) {
        fclose(fp);
        archive_game(plid, "F");
    }
    return;
}

void quit(char *response, char *plid) {
    /*
     * 2. Search for active game ? return "NOK"
     * 3. End game and save score
     * 4. Archive active game
     * 5. return "OK"
     */
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    int code;
    char state[] = "Q";

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code != 1) {
        strcpy(response, "RQT NOK\n");
        return;
    }
    if (archive_game(plid, state)) {
        strcpy(response, "RQT ERR\n");
        return;
    }
    strcpy(response, "RQT OK\n");
    return;
}

/* Returns 0 if there's no response to send.*/
int rev(char *response, char *plid) {
    /*
     * 0. Invalid sintax ? return "ERR"
     * 1. Invalid plid ? return "ERR"
     * 2. Search for active game ? no return
     * 3. return "word"
     */
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    int code;
    char word[WORD_MAX + 1];
    FILE *fp;

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code != 1)
        return 0;
    fp = fopen(active_game_path, "r");
    fscanf(fp, "%s", word);
    fclose(fp);
    snprintf(response, CMD_ID_LEN + 1 + WORD_MAX + 2, "RRV %s\n", word);
    return 1;
}

int get_top_10_scores(Score *score_list) {
    struct dirent **filelist;
    int n_entries;
    char fname[SCORE_FILE_PATH_LEN + 1];
    FILE *fp;
    Score s;
    int i = 0;

    n_entries = scandir(SCORES_DIR, &filelist, 0, alphasort);
    n_entries--;
    if (n_entries == 1) {
        return 0;
    }
    for (; n_entries > 0; n_entries--) {
        if (filelist[n_entries]->d_name[0] == '.') {
            free(filelist[n_entries]);
            continue;
        }
        snprintf(fname, SCORE_DIR_LEN + SCORE_FILE_PATH_LEN + 1, "SCORES/%s", filelist[n_entries]->d_name);
        fp = fopen(fname, "r");
        if (fp == NULL)
            continue;
        s = score_list[i];
        fscanf(fp, "%d %s %s %d %d", &s.score, s.plid, s.word, &s.right_guesses, &s.total);
        fclose(fp);
        i++;
        free(filelist[n_entries]);
        if (i == 10)
            break;
    }
    free(filelist);
    return i;
}

int get_scoreboard(char *response, pid_t pid, FILE **fp) {
    /*
     * 1. Find top 10 scores in SCORES/
     * 2. Is empty ? return status = EMPTY
     * 3. Build Fname = TOPSCORES_nnnnnnn.txt, with nnnnnnn being the pid
     * 4. Parse to pretended format and count chars
     * 5. Build Fsize = chars counted
     * 6. Send to socket the data + '\n'
     */
    char scoreboard[SCOREBOARD_LINE_LEN * 10 + 1];
    char *ptr = scoreboard;
    char fname[FNAME_LEN + 1];
    int fsize;
    Score score_list[10];
    int n;

    n = get_top_10_scores(score_list);
    if (n == 0) {
        strcpy(response, "RSB EMPTY\n");
        return 0;
    }
    for (int i = 0; i < n; i++) {
        ptr += snprintf(ptr, SCOREBOARD_LINE_LEN, "%03d %s %s %d %d\n",
                        score_list[i].score, score_list[i].plid, score_list[i].word,
                        score_list[i].right_guesses, score_list[i].total);
    }
    *(ptr + 1) = '\0';
    snprintf(fname, FNAME_LEN + 1, "TOPSCORES_%07d.txt", pid);
    fsize = ptr - scoreboard;
    *fp = tmpfile();
    fprintf(*fp, scoreboard);
    fflush(*fp);
    rewind(*fp);
    snprintf(response, CMD_ID_LEN + 1 + 2 + 1 + FNAME_LEN + 1 + FSIZE_LEN + 2,
             "RSB OK %s %d ", fname, fsize);
    return fsize;
}

int get_hint_image(char *response, char *plid, FILE **fp) {
    /*
     * 2. Search for active game ? return "NOK"
     * 3. Get hint fname
     * 4. Get file size
     * 5. Send directly from file to socket + '\n'
     */
    int code;
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char header[GAME_FILE_HEADER_MAX + 1];
    char hint[FNAME_LEN + 1];
    int fsize;

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code != 1) {
        strcpy(response, "RHL NOK\n");
        return 0;
    }
    *fp = fopen(active_game_path, "r");
    fgets(header, GAME_FILE_HEADER_MAX + 1, *fp);
    sscanf(header, "%*s %s", hint);
    fseek(*fp, 0, SEEK_END);
    fsize = ftell(*fp);
    rewind(*fp);
    snprintf(response, CMD_ID_LEN + 1 + 2 + 1 + FNAME_LEN + 1 + FSIZE_LEN + 2,
             "RHL OK %s %d ", hint, fsize);
    return fsize;
}

int get_state(char *response, char *plid, FILE **fp) {
    /*
     * 2. Search for active game or last recently played
     * 3. Get game file name
     * 4. Get game file size
     * 5. Send directly from file to socket + '\n'
     */
    int code, fsize;
    char active_game_path[ACTIVE_GAME_PATH_LEN + 1];
    char archived_game_path[ARCHIVED_GAME_PATH_LEN + 1];
    char fname[FNAME_LEN + 1];

    code = find_last_game(plid, active_game_path, archived_game_path);
    if (code == -1) {
        strcpy(response, "RST ERR\n");
        return 0;
    } else if (code == 0) {
        sscanf(archived_game_path, "%*s/%*s/%s", fname);
        *fp = fopen(archived_game_path, "r");
    } else {
        sscanf(active_game_path, "%*s/%s", fname);
        *fp = fopen(active_game_path, "r");
    }
    fseek(*fp, 0, SEEK_END);
    fsize = ftell(*fp);
    rewind(*fp);
    snprintf(response, CMD_ID_LEN + 1 + 2 + 1 + FNAME_LEN + 1 + FSIZE_LEN + 2,
             "RST OK %s %d", fname, fsize);
    return fsize;
}