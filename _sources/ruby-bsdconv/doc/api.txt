Ruby binding API
==================

.. rb:module:: bsdconv

.. rb:class:: Bsdconv

	.. rb:classmethod:: new(conversion)

		Create converter instance with given conversion string

	.. rb:classmethod:: insert_phase(conversion, codec, phase_type, phasen)

		Manipulate conversion string

	.. rb:classmethod:: insert_codec(conversion, codec, phasen, codecn)

		Manipulate conversion string

	.. rb:classmethod:: replace_phase(conversion, codec, phase_type, phasen)

		Manipulate conversion string

	.. rb:classmethod:: replace_codec(conversion, codec, phasen, codecn)

		Manipulate conversion string

	.. rb:classmethod:: error()

		Get error message

	.. rb:classmethod:: codecs_list(phase_type)

		Get codecs list

	.. rb:classmethod:: codec_check(phase_type, codec)

		Check codec availability with given phase type and codec name

	.. rb:classmethod:: mktemp(template)

		mkstemp()

	.. rb:classmethod:: fopen(path, mode)

		fopen()

	.. rb:method:: conv(s)

		Perform conversion

	.. rb:method:: init()

		Initialize/Reset bsdconv converter

	.. rb:method:: ctl(arg_ptr_obj, arg_int)

		Manipulate the underlying codec parameters

	.. rb:method:: conv_chunk(s)

		Perform conversion without initializing and flushing

	.. rb:method:: conv_chunk_last(s)

		Perform conversion without initializing

	.. rb:method:: conv_file(from_file, to_file)

		Perform conversion with given filename

	.. rb:method:: counter([name])

		Return conversion info

	.. rb:method:: counter_reset([name])

		Reset counter, if no name supplied, all counters will be reset
