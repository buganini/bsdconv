C API
=====

.. c:function:: struct bsdconv_instance * bsdconv_create(const char *conversion)

	Create converter instance with given conversion

.. c:function:: int bsdconv_get_phase_index(struct bsdconv_instance *converter, int phase)
.. c:function:: int bsdconv_get_codec_index(struct bsdconv_instance *converter, int phase, int codec)
.. c:function:: char * bsdconv_insert_phase(const char *conversion, const char *conversion_phase, int phase_type, int phase)

	Insert conversion phase into bsdconv conversion string

.. c:function:: char * bsdconv_insert_codec(const char *conversion, const char *conversion_codec, int phase, int codec)

	Insert conversion codec into bsdconv conversion string

.. c:function:: char * bsdconv_replace_phase(const char *conversion, const char *conversion_phase, int phase_type, int phase)

	Replace conversion phase in the bsdconv conversion string

.. c:function:: char * bsdconv_replace_codec(const char *conversion, const char *conversion_codec, int phase, int codec)

	Replace conversion codec in the bsdconv conversion string

.. c:function:: void bsdconv_init(struct bsdconv_instance *converter)

	Initialize/Reset bsdconv converter

.. c:function:: void bsdconv_ctl(struct bsdconv_instance *converter, int ctl, void *pointer, int value)

	Manipulate the underlying codec parameters

.. c:function:: void bsdconv_destroy(struct bsdconv_instance *converter)

	Destroy converter instance

.. c:function:: void bsdconv(struct bsdconv_instance *converter)

	Perform conversion

.. c:function:: char * bsdconv_error(void)

	Get error message

.. c:function:: bsdconv_counter_t * bsdconv_counter(struct bsdconv_instance *converter, const char *name)

	Get pointer to counter

.. c:function:: void bsdconv_counter_reset(struct bsdconv_instance *converter, const char *name)

	Reset counter, if name is NULL, all counters will be reset

.. c:function:: void bsdconv_hash_set(struct bsdconv_instance *converter, const char *key, void *pointer)
.. c:function:: void * bsdconv_hash_get(struct bsdconv_instance *converter, const char *key)
.. c:function:: int bsdconv_hash_has(struct bsdconv_instance *converter, const char *key)
.. c:function:: void bsdconv_hash_del(struct bsdconv_instance *converter, const char *key)

	Access key-value system stored in conversion instance

.. c:function:: char * bsdconv_solve_alias(int type, char *name)

	Solve module alias

.. c:function:: int bsdconv_codec_check(int type, const char *module)

	DEPRECATED: use :c:func:`bsdconv_module_check()` instead

.. c:function:: int bsdconv_module_check(int type, const char *module)

	Check availability with given type and module name

.. c:function:: char ** bsdconv_codecs_list(int type)

	DEPRECATED: use :c:func:`bsdconv_modules_list()` instead

.. c:function:: char ** bsdconv_modules_list(int type)

	Get modules list of specified type

.. c:function:: char *bsdconv_pack(struct bsdconv_instance *converter)

	Get equivalent conversion string of a conversion instance

.. c:function:: void *bsdconv_malloc(size_t size)

	malloc()

.. c:function:: void bsdconv_free(void *ptr)

	free()

.. c:function:: int bsdconv_mkstemp(char *template)

	mkstemp()

.. c:function:: int str2datum(const char *string, struct data_rt *)

	Parse single data cell

.. c:function:: struct data_rt * str2data(const char *string, int *, struct bsdconv_instance *converter)

	Parse data list

.. c:function:: char * getCodecDir()

	Get codec search path
