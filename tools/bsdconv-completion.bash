# bash completion for bsdconv

_bsdconv()
{
    local prev cur i

    COMPREPLY=()
    _get_comp_words_by_ref -n ",:|" cur prev

    case $prev in
        -l)
            return 0
            ;;
    esac


    # if '-i' is already given, complete all kind of files.
    for (( i=0; i < ${#COMP_WORDS[@]}-1; i++ )); do
        if [[ ${COMP_WORDS[i]} == -i ]]; then
            _filedir
        fi
    done

    if [[ $prev == bsdconv ]]; then
        COMPREPLY=( $( bsdconv_completion "$cur" | sort ) )
    else
        _filedir
    fi

    return 0
} &&
complete -o nospace -F _bsdconv bsdconv
