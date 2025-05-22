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

    set literals "--help" "gif" "-d" "--ascii" "--format" "mp4" "-o" "-v" "-F" "--version" "--export" "--output" "--no-sandbox" "--duration" "-h" "-x" "gif" "-A" "-S" "mp4"

    set descriptions
    set descriptions[3] "Exit after the specified amount of seconds"
    set descriptions[7] "Specify a different output path"
    set descriptions[8] "Prints the current version and exits"
    set descriptions[9] "Specify the format of the exported file"
    set descriptions[15] "Prints the help message"
    set descriptions[16] "Export the render in a media file"
    set descriptions[18] "Render visuals in the terminal"
    set descriptions[19] "Do not sandbox lua (Be careful)"

    set literal_transitions
    set literal_transitions[2] "set inputs 1 3 4 5 7 8 9 10 12 14 13 11 15 16 18 19; set tos 4 4 4 5 3 4 6 4 3 4 4 4 4 4 4 4"
    set literal_transitions[4] "set inputs 1 3 4 5 7 8 9 10 12 14 13 11 15 16 18 19; set tos 4 4 4 5 3 4 6 4 3 4 4 4 4 4 4 4"
    set literal_transitions[5] "set inputs 17 20; set tos 4 4"
    set literal_transitions[6] "set inputs 17 20; set tos 4 4"

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
