Go binding API
==================

.. go:package:: bsdconv

.. go:type:: Bsdconv

.. go:function:: func Create(s string)(*Bsdconv)

	Create converter instance with given conversion string

.. go:function:: func (this Bsdconv) String()(string)

.. go:function:: func (this Bsdconv) Init()

.. go:function:: func (this Bsdconv) Conv_chunk(b []byte)([]byte)

.. go:function:: func (this Bsdconv) Conv_chunk_last(b []byte)([]byte)

.. go:function:: func (this Bsdconv) Conv(b []byte)([]byte)

.. go:function:: func (this Bsdconv) Conv_file(ifile string, ofile string)

.. go:function:: func (this Bsdconv) Destroy()

.. go:function:: func (this Bsdconv) Counter(ct interface{})(interface{})

.. go:function:: func (this Bsdconv) Ctl(ctl int, p unsafe.Pointer, v int)

.. go:function:: func Insert_phase(conversion string, codec string, phase_type int, phasen int)(string)

.. go:function:: func Insert_codec(conversion string, codec string, phasen int, codecn int)(string)

.. go:function:: func Replace_phase(conversion string, codec string, phase_type int, phasen int)(string)

.. go:function:: func Replace_codec(conversion string, codec string, phasen int, codecn int)(string)

.. go:function:: func Codec_check(t int, c string)(bool)

.. go:function:: func Codecs_list(t int)([]string)

.. go:function:: func Mktemp(template string)(*C.FILE, string)

.. go:function:: func Fopen(p string, m string)(*C.FILE)

.. go:function:: func Fclose(fp *C.FILE)
