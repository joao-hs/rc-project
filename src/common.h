#define TRUE 1
#define FALSE 0
#define CMD_ID_LEN 3
#define STATUS_LEN 3
#define PLID_LEN 6
#define WORD_MIN 3
#define WORD_MAX 30
#define FNAME_LEN 24
#define FSIZE_LEN 10
#define FDATA_MAX 1073741824
#define MAX_PLID 6
#define MAX_HOST 253
#define MAX_PORT 6
#define MAX_MESSAGE CMD_ID_LEN + 1 + WORD_MAX + 1
#define MAX_UDP_RESPONSE CMD_ID_LEN + 1 + STATUS_LEN + 1 + 2 + 2 + WORD_MAX*2 
#define MAX_TCP_RESPONSE CMD_ID_LEN + 1 + STATUS_LEN + 1 + FNAME_LEN + 1 + FSIZE_LEN + 1 + FDATA_MAX + 1


#define MAX(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x > _y ? _x : _y;})
#define MIN(x, y) ({__typeof__ (x) _x = (x); __typeof__ (y) _y = (y); _x <= _y ? _x : _y;})

