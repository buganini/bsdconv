enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,	
};

struct state_s{
	char status;
	void *data;
	size_t len;
	struct *state_s[257];
};
