//define DEBUG
#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("DEBUG: %d " fmt "\n", __LINE__, ## args); fflush(stdout);
#else
#define DPRINTF(fmt, args...)
#endif

#include <unistd.h>

struct bsdconv_codec_t {
	char *desc;
	int fd;
	unsigned char *z;
};

struct bsdconv_t {
	int nfrom;
	int ninter;
	int nto;
	struct bsdconv_codec_t *from;
	struct bsdconv_codec_t *inter;
	struct bsdconv_codec_t *to;
};

enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,
	SUBMATCH,
	CALLBACK,
};

struct data_s{
	unsigned char *data;
	size_t len;
	struct data_s *next;
};

struct state_s{
	char status;
	struct data_s *data;
	struct state_s *sub[257];
};

struct bsdconv_instruction{
	unsigned char *in_buf;
	size_t in_len;
	unsigned char *out_buf;
	size_t out_len;
	unsigned char *feed;
	size_t feed_len;
	unsigned char *back;
	size_t back_len;

	unsigned char pend_from, pend_inter, pend_to;

	unsigned char ierr, oerr;

	struct state_s from_state, inter_state, to_state;
	int from_index, inter_index, to_index;
	struct state_s from_match, inter_match, to_match;
	struct data_s *inter_data_head, *to_data_head, *out_data_head, **inter_data_tail, **to_data_tail, **out_data_tail;
};

void bsdconv_init(struct bsdconv_t *, struct bsdconv_instruction *, unsigned char *, size_t, unsigned char *, size_t);
struct bsdconv_t *bsdconv_create(const char *);
void bsdconv_destroy(struct bsdconv_t *);
int bsd_conv(struct bsdconv_t *, struct bsdconv_instruction *);

