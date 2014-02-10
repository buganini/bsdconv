#compdef bsdconv

_bsdconv() {

	_arguments \
		'1: :->first'\
		'2: :->second'\
		'*: :->files'

	case $state in
	first)
		if [ "$words[2]" = "-" ]
		then
			compadd -- "-l"
		else
			compadd `bsdconv-completion "$words[2]"`
		fi
	;;
	second)
		if [ "$words[3]" = "-" ]
		then
			compadd -- "-i"
		else
			_files
		fi
	;;
	files)
		_files
	;;
	esac
}

_bsdconv "$@"
