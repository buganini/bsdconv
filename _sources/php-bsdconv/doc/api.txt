PHP binding API
==================

.. php:const:: BSDCONV_FROM
.. php:const:: BSDCONV_INTER
.. php:const:: BSDCONV_TO

	Phase type

.. php:const:: BSDCONV_CTL_ATTACH_SCORE
.. php:const:: BSDCONV_CTL_ATTACH_OUTPUT_FILE
.. php:const:: BSDCONV_CTL_AMBIGUOUS_PAD

	Request for :php:func:`Bsdconv::ctl`

.. php:function:: bsdconv_insert_phase($conversion, $codec, $phase_type, $phasen)

		Insert conversion phase into bsdconv conversion string

.. php:function:: bsdconv_insert_codec($conversion, $codec, $phasen, $codecn)

		Insert conversion codec into bsdconv conversion string

.. php:function:: bsdconv_replace_phase($conversion, $codec, $phase_type, $phasen)

		Replace conversion phase in the bsdconv conversion string

.. php:function:: bsdconv_replace_codec($conversion, $codec, $phasen, $codecn)

		Replace conversion codec in the bsdconv conversion string

.. php:function:: bsdconv_error()

	Get error message

.. php:function:: bsdconv_codecs_list($phase_type)

	Get codecs list

.. php:function:: bsdconv_codec_check($phase_type, $codec)

	Check codec availability with given phase type and codec name

.. php:function:: bsdconv_fopen($path, $mode)

	fopen()

.. php:function:: bsdconv_fclose($fp)

	fclose()

.. php:function:: bsdconv_mktemp($template)

	mkstemp()

.. php:class:: Bsdconv

	.. php:method:: conv($s)

		Perform conversion

	.. php:method:: init()

		Initialize/Reset bsdconv converter

	.. php:method:: ctl($p, $v)

		Manipulate the underlying codec parameters

	.. php:method:: conv_chunk($s)

		Perform conversion without initializing and flushing

	.. php:method:: conv_chunk_last($s)

		Perform conversion without initializing

	.. php:method:: conv_file($from_file, $to_file)

		Perform conversion with given filename

	.. php:method:: counter([$name])

		Return conversion info

	.. php:method:: counter_reset([$name])

		Reset counter, if no name supplied, all counters will be reset

