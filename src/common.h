#define TRUE 1
#define FALSE 0
#define MAXMESSAGE CMD_ID_LEN + 1 + WORD_MAX + 1
#define CMD_ID_LEN 3
#define FEED_ID_LEN 3
#define STATUS_LEN 3
#define PLID_LEN 6
#define WORD_MIN 3
#define WORD_MAX 30
#define MAX_PLID 6
#define MAXHOST 253
#define MAXPORT 6


#define MAX(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x > _y ? _x : _y;})
#define MIN(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x <= _y ? _x : _y;})

