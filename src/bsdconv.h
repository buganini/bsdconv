enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,	
};

struct data_s{
	unsigned char *data;
	size_t len;
};

struct state_s{
	int status;
	unsigned char *data;
	unsigned char *sub[257];
};
