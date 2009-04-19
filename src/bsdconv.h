//define DEBUG
#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("DEBUG: %d " fmt "\n", __LINE__, ## args); fflush(stdout);
#else
#define DPRINTF(fmt, args...)
#endif

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
};

struct data_s{
	unsigned int p;
	unsigned int data;
	size_t len;
	unsigned int next;
};

struct state_s{
	char status;
	unsigned int data;
	unsigned int sub[257];
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
	unsigned char *inter_d, *to_d, *out_d, *inter_z, *to_z, *out_z;
	struct state_s from_match, inter_match, to_match;
	struct data_s inter_data, to_data, out_data;
};

void bsdconv_init(struct bsdconv_t *, struct bsdconv_instruction *, unsigned char *, size_t, unsigned char *, size_t);
struct bsdconv_t *bsdconv_create(const char *);
void bsdconv_destroy(struct bsdconv_t *);
int bsd_conv(struct bsdconv_t *, struct bsdconv_instruction *);

