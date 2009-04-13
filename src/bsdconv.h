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
};

struct data_s{
	int p;
	int data;
	size_t len;
	int next;
};

struct state_s{
	int status;
	int data;
	int sub[257];
};

struct bsdconv_instruction{
	char *in_buf;
	size_t in_len;
	char *out_buf;
	size_t out_len;
	char *feed;
	size_t feed_len;
	char *back;
	size_t back_len;
};

void bsdconv_init(struct bsdconv_t *, struct bsdconv_instruction *, char *, size_t, char *, size_t);
struct bsdconv_t *bsdconv_create(const char *, const char *, const char *);
void bsdconv_destroy(struct bsdconv_t *);
int bsd_conv(struct bsdconv_t *, struct bsdconv_instruction *);

