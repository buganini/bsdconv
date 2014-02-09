#compdef bsdconv

_bsdconv() {
	_arguments \
		'1: :->conversion'\
		'*: :->files'

	case $state in
	conversion)
		compadd `bsdconv-completion "$words[2]"`
	;;
	files)
		_files
	;;
	esac
}

_bsdconv "$@"
