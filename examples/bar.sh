#!/bin/bash
ff="/tmp/howm.fifo"
logfile="/home/harvey/.config/howm/log.txt"
[[ -p $ff ]] || mkfifo -m 666 "$ff"

ws=("term" "vim" "www" "chat" "media")

lay=("▣" "▦" "▥" "▤")

mbg=("#333333" "#5F5F87" "#AFD7AF")
mfg=("#DDDDDD" "#333333" "#333333")

bg="#333333"

while read -t 10 -r howmout || true; do
    if [[ $howmout =~ ^(([[:digit:]]+:)+[[:digit:]]+ ?)+$ ]]; then
        unset r
        IFS=':' read -r m l w s c <<< "$howmout"
        r+='^fg('"${mfg[$m]}"')'
        r+='^bg('"${mbg[$m]}"')'
        r+=" ${lay[$l]} | "
        r+="${ws[$w - 1]}"
        r="${r%::*}"
    fi
    printf "%s%s\n" "$r" " | $(date +"%F %R") "
done < "$ff" | dzen2 -h 20 -y -20 -ta r -bg "$bg" -fn "Inconsolata-dz:size=10" &

# pass output to fifo
/usr/bin/howm -c ~/.config/howm/howmrc 1> "$ff" 2> "$logfile"
