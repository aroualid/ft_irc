#!/bin/bash


SESSION_NAME="irssi-panes"
SERVER="localhost"
PORT="6667"
PASSWORD="testpass"
NICK_PREFIX="lab-rat-"

if tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
    echo "Session '$SESSION_NAME' already exists. Killing it."
    tmux kill-session -t "$SESSION_NAME"
fi

PASSWORD_ARG=""
if [ -n "$PASSWORD" ]; then
    PASSWORD_ARG="-w $PASSWORD"
fi

echo "Creating new session '$SESSION_NAME' with the first irssi pane..."
tmux new-session -d -s "$SESSION_NAME" "irssi -c $SERVER -p $PORT -n ${NICK_PREFIX}1 $PASSWORD_ARG"

for i in {2..6}; do
    NICK="${NICK_PREFIX}${i}"
    echo "Creating pane for $NICK..."
    tmux split-window -d "irssi -c $SERVER -p $PORT -n $NICK $PASSWORD_ARG"
done

echo "Arranging panes in a tiled layout..."
tmux select-layout -t "$SESSION_NAME:0" tiled

echo "All irssi panes created."
echo "Attaching to session '$SESSION_NAME'..."
tmux attach-session -t "$SESSION_NAME"
