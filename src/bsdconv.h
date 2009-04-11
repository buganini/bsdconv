enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,	
};

struct data_s{
	void *data;
	size_t len;
}

struct state_s{
	char status;
	struct data_s *data;
	int sub[257];
};
