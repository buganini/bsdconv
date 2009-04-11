enum bsdconv_status{
	CONTINUE,
	DEADEND,
	MATCH,	
};

struct data_s{
	int data;
	size_t len;
	int next;
};

struct state_s{
	int status;
	int data;
	int sub[257];
};
