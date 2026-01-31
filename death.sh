#!/usr/bin/env bash

if [[ $# -lt 3 ]]; then
	echo "Usage: $0 <hostname> <port> <password>"
	echo "Example: $0 127.0.0.1 6667 lol"
	exit 1
fi

HOST="$1"
PORT="$2"
PASS="$3"

CLIENTS=1000
CHANNEL="#the-void"
MAX_DEATHS=10
MESSAGE="this is a test message"
SLEEP_BETWEEN_MSGS=0.01   # set to 0 for max pain

rand_nick() {
	echo "lab$(tr -dc a-z0-9 </dev/urandom | head -c6)"
}

rand_goodbye() {
	shuf -n1 <<-EOF
	brb dissolving
	lost to entropy
	lab rat escaped
	signal collapsed
	EOF
}

# choose victims
DEATH_COUNT=$(shuf -i0-$MAX_DEATHS -n1)
DEAD_CLIENTS=$(shuf -i1-$CLIENTS -n "$DEATH_COUNT")

should_die() {
	grep -qx "$1" <<< "$DEAD_CLIENTS"
}

echo "[*] Target: $HOST:$PORT"
echo "[*] Clients: $CLIENTS"
echo "[*] Mid-stream deaths: $DEATH_COUNT"

for i in $(seq 1 "$CLIENTS"); do
	(
		nick=$(rand_nick)
		msg_count=$(shuf -i100-200 -n1)

		{
			echo "PASS $PASS"
			echo "NICK $nick"
			echo "USER $nick 0 * : Lab rat"
			echo "JOIN $CHANNEL"

			for m in $(seq 1 "$msg_count"); do
				echo "PRIVMSG $CHANNEL :$MESSAGE [$m/$msg_count]"
				sleep "$SLEEP_BETWEEN_MSGS"

				if should_die "$i" && (( m == msg_count / 2 )); then
					# abrupt disconnect (no QUIT)
					exit 0
				fi
			done

			echo "QUIT :$(rand_goodbye)"
		} | nc "$HOST" "$PORT" >/dev/null 2>&1
	) &
done

wait
echo "[*] Tests complete, surely your server hasn't died right :)?"
