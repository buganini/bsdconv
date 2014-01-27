#include "../../src/bsdconv.h"

int cbfilter(struct data_rt *data){
	if(data->len>0 && UCP(data->data)[0]==TYPE)
		return 1;
	else
		return 0;
}
