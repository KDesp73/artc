#compdef artc

_artc_cmd_0 () {
    _path_files
}

_artc () {
    local -a literals=("--help" "gif" "png" "--ascii" "--format" "mp4" "-o" "-d" "-v" "-F" "--version" "png" "--export" "--output" "--no-sandbox" "-h" "-x" "gif" "-A" "-S" "--duration" "mp4")

    local -A descriptions
    descriptions[7]="Specify a different output path"
    descriptions[8]="Exit after the specified amount of seconds"
    descriptions[9]="Prints the current version and exits"
    descriptions[10]="Specify the format of the exported file"
    descriptions[16]="Prints the help message"
    descriptions[17]="Export the render in a media file"
    descriptions[19]="Render visuals in the terminal"
    descriptions[20]="Do not sandbox lua (Be careful)"

    local -A literal_transitions
    literal_transitions[2]="([1]=4 [8]=4 [4]=4 [5]=5 [7]=3 [21]=4 [10]=6 [11]=4 [14]=3 [9]=4 [15]=4 [13]=4 [16]=4 [17]=4 [19]=4 [20]=4)"
    literal_transitions[4]="([1]=4 [8]=4 [4]=4 [5]=5 [7]=3 [21]=4 [10]=6 [11]=4 [14]=3 [9]=4 [15]=4 [13]=4 [16]=4 [17]=4 [19]=4 [20]=4)"
    literal_transitions[5]="([12]=4 [18]=4 [22]=4)"
    literal_transitions[6]="([18]=4 [22]=4 [12]=4)"

    local -A match_anything_transitions
    match_anything_transitions=([3]=4 [1]=2)

    declare -A subword_transitions

    local state=1
    local word_index=2
    while [[ $word_index -lt $CURRENT ]]; do
        if [[ -v "literal_transitions[$state]" ]]; then
            local -A state_transitions
            eval "state_transitions=${literal_transitions[$state]}"

            local word=${words[$word_index]}
            local word_matched=0
            for ((literal_id = 1; literal_id <= $#literals; literal_id++)); do
                if [[ ${literals[$literal_id]} = "$word" ]]; then
                    if [[ -v "state_transitions[$literal_id]" ]]; then
                        state=${state_transitions[$literal_id]}
                        word_index=$((word_index + 1))
                        word_matched=1
                        break
                    fi
                fi
            done
            if [[ $word_matched -ne 0 ]]; then
                continue
            fi
        fi

        if [[ -v "match_anything_transitions[$state]" ]]; then
            state=${match_anything_transitions[$state]}
            word_index=$((word_index + 1))
            continue
        fi

        return 1
    done
    declare -A literal_transitions_level_0=([2]="8 7 10 9 16 17 19 20" [6]="18 22 12" [4]="8 7 10 9 16 17 19 20")
    declare -A literal_transitions_level_1=([2]="1 4 5 21 11 14 15 13" [5]="12 18 22" [4]="1 4 5 21 11 14 15 13")
    declare -A subword_transitions_level_0=()
    declare -A subword_transitions_level_1=()
    declare -A commands_level_0=()
    declare -A commands_level_1=()
    declare -A specialized_commands_level_0=([3]="0" [1]="0")
    declare -A specialized_commands_level_1=()

     local max_fallback_level=1
     for (( fallback_level=0; fallback_level <= max_fallback_level; fallback_level++ )) {
         completions_no_description_trailing_space=()
         completions_no_description_no_trailing_space=()
         completions_trailing_space=()
         suffixes_trailing_space=()
         descriptions_trailing_space=()
         completions_no_trailing_space=()
         suffixes_no_trailing_space=()
         descriptions_no_trailing_space=()
         matches=()

         declare literal_transitions_name=literal_transitions_level_${fallback_level}
         eval "declare initializer=\${${literal_transitions_name}[$state]}"
         eval "declare -a transitions=($initializer)"
         for literal_id in "${transitions[@]}"; do
             if [[ -v "descriptions[$literal_id]" ]]; then
                 completions_trailing_space+=("${literals[$literal_id]}")
                 suffixes_trailing_space+=("${literals[$literal_id]}")
                 descriptions_trailing_space+=("${descriptions[$literal_id]}")
             else
                 completions_no_description_trailing_space+=("${literals[$literal_id]}")
             fi
         done

         declare subword_transitions_name=subword_transitions_level_${fallback_level}
         eval "declare initializer=\${${subword_transitions_name}[$state]}"
         eval "declare -a transitions=($initializer)"
         for subword_id in "${transitions[@]}"; do
             _artc_subword_${subword_id} complete "${words[$CURRENT]}"
             completions_no_description_trailing_space+=("${subword_completions_no_description_trailing_space[@]}")
             completions_trailing_space+=("${subword_completions_trailing_space[@]}")
             completions_no_trailing_space+=("${subword_completions_no_trailing_space[@]}")
             suffixes_no_trailing_space+=("${subword_suffixes_no_trailing_space[@]}")
             suffixes_trailing_space+=("${subword_suffixes_trailing_space[@]}")
             descriptions_trailing_space+=("${subword_descriptions_trailing_space[@]}")
             descriptions_no_trailing_space+=("${subword_descriptions_no_trailing_space[@]}")
         done

         declare commands_name=commands_level_${fallback_level}
         eval "declare initializer=\${${commands_name}[$state]}"
         eval "declare -a transitions=($initializer)"
         for command_id in "${transitions[@]}"; do
             local output=$(_artc_cmd_${command_id} "${words[$CURRENT]}")
             local -a command_completions=("${(@f)output}")
             for line in ${command_completions[@]}; do
                 local parts=(${(@s:	:)line})
                 if [[ -v "parts[2]" ]]; then
                     completions_trailing_space+=("${parts[1]}")
                     suffixes_trailing_space+=("${parts[1]}")
                     descriptions_trailing_space+=("${parts[2]}")
                 else
                     completions_no_description_trailing_space+=("${parts[1]}")
                 fi
             done
         done

         declare specialized_commands_name=specialized_commands_level_${fallback_level}
         eval "declare initializer=\${${specialized_commands_name}[$state]}"
         eval "declare -a transitions=($initializer)"
         for command_id in "${transitions[@]}"; do
             _artc_cmd_${command_id} ${words[$CURRENT]}
         done

         local maxlen=0
         for suffix in ${suffixes_trailing_space[@]}; do
             if [[ ${#suffix} -gt $maxlen ]]; then
                 maxlen=${#suffix}
             fi
         done
         for suffix in ${suffixes_no_trailing_space[@]}; do
             if [[ ${#suffix} -gt $maxlen ]]; then
                 maxlen=${#suffix}
             fi
         done

         for ((i = 1; i <= $#suffixes_trailing_space; i++)); do
             if [[ -z ${descriptions_trailing_space[$i]} ]]; then
                 descriptions_trailing_space[$i]="${(r($maxlen)( ))${suffixes_trailing_space[$i]}}"
             else
                 descriptions_trailing_space[$i]="${(r($maxlen)( ))${suffixes_trailing_space[$i]}} -- ${descriptions_trailing_space[$i]}"
             fi
         done

         for ((i = 1; i <= $#suffixes_no_trailing_space; i++)); do
             if [[ -z ${descriptions_no_trailing_space[$i]} ]]; then
                 descriptions_no_trailing_space[$i]="${(r($maxlen)( ))${suffixes_no_trailing_space[$i]}}"
             else
                 descriptions_no_trailing_space[$i]="${(r($maxlen)( ))${suffixes_no_trailing_space[$i]}} -- ${descriptions_no_trailing_space[$i]}"
             fi
         done

         compadd -O m -a completions_no_description_trailing_space; matches+=("${m[@]}")
         compadd -O m -a completions_no_description_no_trailing_space; matches+=("${m[@]}")
         compadd -O m -a completions_trailing_space; matches+=("${m[@]}")
         compadd -O m -a completions_no_trailing_space; matches+=("${m[@]}")

         if [[ ${#matches} -gt 0 ]]; then
             compadd -Q -a completions_no_description_trailing_space
             compadd -Q -S ' ' -a completions_no_description_no_trailing_space
             compadd -l -Q -a -d descriptions_trailing_space completions_trailing_space
             compadd -l -Q -S '' -a -d descriptions_no_trailing_space completions_no_trailing_space
             return 0
         fi
     }
}

if [[ $ZSH_EVAL_CONTEXT =~ :file$ ]]; then
    compdef _artc artc
else
    _artc
fi
