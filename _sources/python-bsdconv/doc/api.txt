Python binding API
==================

.. py:module:: bsdconv

.. py:class:: Bsdconv

	.. py:data:: FROM
	.. py:data:: INTER
	.. py:data:: TO

		Phase type

	.. py:data:: CTL_ATTACH_SCORE
	.. py:data:: CTL_ATTACH_OUTPUT_FILE
	.. py:data:: CTL_AMBIGUOUS_PAD

		Request for :py:meth:`ctl`

	.. py:method:: init()

		Initialize/Reset bsdconv converter

	.. py:method:: ctl(arg_ptr_obj, arg_int)

		Manipulate the underlying codec parameters

	.. py:method:: conv(s)

		Perform conversion

	.. py:method:: conv_chunk(s)

		Perform conversion without initializing and flushing

	.. py:method:: conv_chunk_last(s)

		Perform conversion without initializing

	.. py:method:: conv_file(from_file, to_file)

		Perform conversion with given filename

	.. py:method:: testconv(s)

		Perform test conversion

	.. py:method:: testconv_chunk(s)

		Perform test conversion without initializing and flushing

	.. py:method:: testconv_chunk_last(s)

		Perform test conversion without initializing

	.. py:method:: testconv_file(from_file)

		Perform test conversion with given filename

	.. py:method:: counter([name])

		Return counter or counters if not specified

	.. py:method:: counter_reset([name])

		Reset counter, if no name supplied, all counters will be reset

	.. py:staticmethod:: insert_phase(conversion, codecs, phase_type, phasen)

		Insert conversion phase into bsdconv conversion string

	.. py:staticmethod:: insert_codec(conversion, codec, phasen, codecn)

		Insert conversion codec into bsdconv conversion string

	.. py:staticmethod:: replace_phase(conversion, codecs, phase_type, phasen)

		Replace conversion phase in the bsdconv conversion string

	.. py:staticmethod:: replace_codec(conversion, codec, phasen, codecn)

		Replace conversion codec in the bsdconv conversion string

	.. py:staticmethod:: error()

		Return error message

	.. py:staticmethod:: mktemp()

		Make temporary file

	.. py:staticmethod:: fopen()

		Open file

	.. py:staticmethod:: codecs_list()

		list codecs

	.. py:staticmethod:: codec_check(type, codec)

		check if a codec is available
