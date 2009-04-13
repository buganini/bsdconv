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
