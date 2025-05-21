function _artc_spec_2
set 1 $argv[1]
__fish_complete_path "$1"
end

function _artc
    set COMP_LINE (commandline --cut-at-cursor)

    set COMP_WORDS
    echo $COMP_LINE | read --tokenize --array COMP_WORDS
    if string match --quiet --regex '.*\s$' $COMP_LINE
        set COMP_CWORD (math (count $COMP_WORDS) + 1)
    else
        set COMP_CWORD (count $COMP_WORDS)
    end

    set literals "--help" "gif" "--format" "mp4" "-o" "-v" "-F" "--version" "--export" "--output" "-h" "-x" "gif" "mp4"

    set descriptions
    set descriptions[5] "Specify a different output path"
    set descriptions[6] "Prints the current version and exits"
    set descriptions[7] "Specify the format of the exported file"
    set descriptions[11] "Prints the help message"
    set descriptions[12] "Export the render in a media file"

    set literal_transitions
    set literal_transitions[2] "set inputs 1 10 8 12 11 3 5 6 7 9; set tos 4 3 4 4 4 5 3 4 6 4"
    set literal_transitions[4] "set inputs 1 10 8 12 11 3 5 6 7 9; set tos 4 3 4 4 4 5 3 4 6 4"
    set literal_transitions[5] "set inputs 13 14; set tos 4 4"
    set literal_transitions[6] "set inputs 13 14; set tos 4 4"

    set match_anything_transitions_from 3 1
    set match_anything_transitions_to 4 2

    set state 1
    set word_index 2
    while test $word_index -lt $COMP_CWORD
        set -- word $COMP_WORDS[$word_index]

        if set --query literal_transitions[$state] && test -n $literal_transitions[$state]
            set --erase inputs
            set --erase tos
            eval $literal_transitions[$state]

            if contains -- $word $literals
                set literal_matched 0
                for literal_id in (seq 1 (count $literals))
                    if test $literals[$literal_id] = $word
                        set index (contains --index -- $literal_id $inputs)
                        set state $tos[$index]
                        set word_index (math $word_index + 1)
                        set literal_matched 1
                        break
                    end
                end
                if test $literal_matched -ne 0
                    continue
                end
            end
        end

        if set --query match_anything_transitions_from[$state] && test -n $match_anything_transitions_from[$state]
            set index (contains --index -- $state $match_anything_transitions_from)
            set state $match_anything_transitions_to[$index]
            set word_index (math $word_index + 1)
            continue
        end

        return 1
    end

    if set --query literal_transitions[$state] && test -n $literal_transitions[$state]
        set --erase inputs
        set --erase tos
        eval $literal_transitions[$state]
        for literal_id in $inputs
            if test -n $descriptions[$literal_id]
                printf '%s\t%s\n' $literals[$literal_id] $descriptions[$literal_id]
            else
                printf '%s\n' $literals[$literal_id]
            end
        end
    end

    set specialized_command_states 3 1
    set specialized_command_ids 2 2
    if contains $state $specialized_command_states
        set index (contains --index $state $specialized_command_states)
        set function_id $specialized_command_ids[$index]
        set function_name _artc_spec_$function_id
        set --erase inputs
        set --erase tos
        set lines (eval $function_name $COMP_WORDS[$COMP_CWORD])
        for line in $lines
            printf '%s\n' $line
        end
    end

    return 0
end

complete --command artc --no-files --arguments "(_artc)"
