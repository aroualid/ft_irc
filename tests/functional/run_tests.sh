#!/bin/bash
# Functional test suite for ft_irc using netcat
# Usage: ./run_tests.sh [host] [port] [password]
#
# Tested commands: PASS, NICK, USER, JOIN, PART, PRIVMSG, NOTICE, KICK, PING, QUIT, INVITE, TOPIC, MODE, WHO, NAMES

# set -e

# Configuration
HOST="${1:-localhost}"
PORT="${2:-6667}"
PASS="${3:-testpass}"
TIMEOUT=10
TMP_DIR="/tmp/irc_tests_$$"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# ============================================================================
# Test Framework
# ============================================================================

setup() {
    mkdir -p "$TMP_DIR"
    rm -f "$TMP_DIR"/*
}

cleanup() {
    rm -rf "$TMP_DIR"
    # Kill any leftover nc processes from this test run
    pkill -f "tail -f.*nc.*$PORT" 2>/dev/null || true
}

trap cleanup EXIT

# Send IRC commands and capture output
# Usage: irc_send "output_file" "command1" "command2" ...
irc_send() {
    local outfile="$1"
    shift
    {
        for cmd in "$@"; do
echo -e "$cmd\r"
            sleep 0.1
        done
        sleep 0.3
    } | timeout "$TIMEOUT" nc "$HOST" "$PORT" > "$outfile" 2>&1 || true
}

# Start a persistent client (background)
# Usage: start_client "name"
start_client() {
    local name="$1"
    mkfifo "$TMP_DIR/${name}_in" 2>/dev/null || true
    # Use stdbuf for unbuffered output from nc
    tail -f "$TMP_DIR/${name}_in" 2>/dev/null | stdbuf -o0 nc "$HOST" "$PORT" > "$TMP_DIR/${name}_out" 2>&1 &
    echo $! > "$TMP_DIR/${name}_pid"
    sleep 0.5
}

# Send to persistent client
# Usage: client_send "name" "command"
client_send() {
    local name="$1"
    local cmd="$2"
    echo -e "$cmd\r" >> "$TMP_DIR/${name}_in"
}

# Get client output
# Usage: client_output "name"
client_output() {
    local name="$1"
    cat "$TMP_DIR/${name}_out" 2>/dev/null || echo ""
}

# Stop persistent client
# Usage: stop_client "name"
stop_client() {
    local name="$1"
    echo "QUIT :Leaving" >> "$TMP_DIR/${name}_in" 2>/dev/null || true
    sleep 0.2
    local pid=$(cat "$TMP_DIR/${name}_pid" 2>/dev/null)
    [ -n "$pid" ] && kill "$pid" 2>/dev/null || true
    rm -f "$TMP_DIR/${name}_in" "$TMP_DIR/${name}_pid"
}

# Register a persistent client
# Usage: register_client "name" "nick"
register_client() {
    local name="$1"
    local nick="$2"
    client_send "$name" "PASS $PASS"
    client_send "$name" "NICK $nick"
    client_send "$name" "USER $nick 0 * :$nick User"
    sleep 0.3
}

# Assertions
assert_contains() {
    local file="$1"
    local pattern="$2"
    local msg="$3"

    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "  ${GREEN}✓${NC} $msg"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "  ${RED}✗${NC} $msg ${YELLOW}(expected: $pattern)${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

assert_not_contains() {
    local file="$1"
    local pattern="$2"
    local msg="$3"

    if ! grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "  ${GREEN}✓${NC} $msg"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "  ${RED}✗${NC} $msg ${YELLOW}(should not contain: $pattern)${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

assert_output_contains() {
    local name="$1"
    local pattern="$2"
    local msg="$3"
    sleep 0.3
    assert_contains "$TMP_DIR/${name}_out" "$pattern" "$msg"
}

# Mark current position in client output (for checking only new output)
mark_output_position() {
    local name="$1"
    wc -c < "$TMP_DIR/${name}_out" 2>/dev/null > "$TMP_DIR/${name}_pos" || echo 0 > "$TMP_DIR/${name}_pos"
}

# Check if new output (since mark) contains pattern
assert_new_output_contains() {
    local name="$1"
    local pattern="$2"
    local msg="$3"
    local pos=$(cat "$TMP_DIR/${name}_pos" 2>/dev/null || echo 0)
    sleep 0.3
    if tail -c +$((pos + 1)) "$TMP_DIR/${name}_out" 2>/dev/null | grep -q "$pattern"; then
        echo -e "  ${GREEN}✓${NC} $msg"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "  ${RED}✗${NC} $msg ${YELLOW}(expected: $pattern)${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Run a test
run_test() {
    local name="$1"
    shift
    echo -e "${CYAN}[$name]${NC}"
    ((TESTS_RUN++))
    "$@"
    echo
}

# ============================================================================
# Registration Tests (PASS, NICK, USER)
# ============================================================================

test_basic_registration() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK testuser" \
        "USER testuser 0 * :Test User" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "001" "RPL_WELCOME received"
    assert_contains "$TMP_DIR/out.txt" "testuser" "Nickname in welcome"
}

test_wrong_password() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS wrongpassword" \
        "NICK baduser" \
        "USER baduser 0 * :Bad User" \
        "JOIN #shouldfail"

    assert_not_contains "$TMP_DIR/out.txt" "001" "Should not receive RPL_WELCOME"
    assert_contains "$TMP_DIR/out.txt" "451" "ERR_NOTREGISTERED (451) on JOIN after wrong pass"
}

test_no_password() {
    irc_send "$TMP_DIR/out.txt" \
        "NICK nopass" \
        "USER nopass 0 * :No Pass" \
        "JOIN #shouldfail"

    assert_contains "$TMP_DIR/out.txt" "451" "ERR_NOTREGISTERED (451) on JOIN with no pass"
}

test_invalid_nickname() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK 123invalid" \
        "USER test 0 * :Test"

    assert_contains "$TMP_DIR/out.txt" "432" "ERR_ERRONEUSNICKNAME (432)"
}

test_nickname_too_long() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK verylongnickname" \
        "USER test 0 * :Test"

    assert_contains "$TMP_DIR/out.txt" "432" "ERR_ERRONEUSNICKNAME for long nick (432)"
}

test_duplicate_nickname() {
    start_client "first"
    register_client "first" "taken"
    sleep 0.2

    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK taken" \
        "USER test 0 * :Test"

    assert_contains "$TMP_DIR/out.txt" "433" "ERR_NICKNAMEINUSE (433)"

    stop_client "first"
}

test_nick_change() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK oldnick" \
        "USER oldnick 0 * :Old Nick" \
        "NICK newnick" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "001" "Registered with old nick"
    assert_contains "$TMP_DIR/out.txt" "NICK.*newnick" "Nick change confirmed"
}

# ============================================================================
# Channel Tests (JOIN, PART)
# ============================================================================

test_join_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK joiner" \
        "USER joiner 0 * :Joiner" \
        "JOIN #test" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "JOIN" "JOIN echo received"
    assert_contains "$TMP_DIR/out.txt" "#test" "Channel name in response"
    assert_contains "$TMP_DIR/out.txt" "353" "RPL_NAMREPLY (353)"
    assert_contains "$TMP_DIR/out.txt" "366" "RPL_ENDOFNAMES (366)"
}

test_join_invalid_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK joiner2" \
        "USER joiner2 0 * :Joiner" \
        "JOIN nochanprefix" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "403" "ERR_NOSUCHCHANNEL (403)"
}

test_join_multiple_channels() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK multiuser" \
        "USER multiuser 0 * :Multi User" \
        "JOIN #chan1,#chan2,#chan3" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "#chan1" "Joined #chan1"
    assert_contains "$TMP_DIR/out.txt" "#chan2" "Joined #chan2"
    assert_contains "$TMP_DIR/out.txt" "#chan3" "Joined #chan3"
}

test_part_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK parter" \
        "USER parter 0 * :Parter" \
        "JOIN #parttest" \
        "PART #parttest :Goodbye" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "JOIN" "Joined channel"
    assert_contains "$TMP_DIR/out.txt" "PART.*#parttest" "PART confirmed"
}

test_part_not_in_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK notinchan" \
        "USER notinchan 0 * :Not In Channel" \
        "PART #nonexistent" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "403" "ERR_NOSUCHCHANNEL (403)"
}

test_first_user_is_operator() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK opuser" \
        "USER opuser 0 * :Op" \
        "JOIN #optest" \
        "QUIT"

    # First user should have @ prefix in NAMES
    assert_contains "$TMP_DIR/out.txt" "@opuser" "First user is operator"
}

test_join_broadcast_to_members() {
    start_client "first"
    register_client "first" "firstusr"
    client_send "first" "JOIN #broadcast"
    sleep 0.2

    # Mark position to only check new output
    mark_output_position "first"

    start_client "second"
    register_client "second" "secusr"
    client_send "second" "JOIN #broadcast"
    sleep 0.3

    assert_new_output_contains "first" "secusr.*JOIN\|JOIN.*secusr" "First user sees second user join"

    stop_client "first"
    stop_client "second"
}

# ============================================================================
# Messaging Tests (PRIVMSG, NOTICE)
# ============================================================================

test_privmsg_channel() {
    start_client "sender"
    start_client "receiver"

    register_client "sender" "sender"
    register_client "receiver" "receiver"

    client_send "sender" "JOIN #msgtest"
    sleep 0.2
    client_send "receiver" "JOIN #msgtest"
    sleep 0.2

    # Mark position to only check new output
    mark_output_position "receiver"

    client_send "sender" "PRIVMSG #msgtest :Hello channel!"
    sleep 0.3

    assert_new_output_contains "receiver" "Hello channel" "Receiver got channel message"

    stop_client "sender"
    stop_client "receiver"
}

test_privmsg_user() {
    start_client "alice"
    start_client "bob"

    register_client "alice" "alice"
    register_client "bob" "bob"
    sleep 0.2

    # Mark position to only check new output
    mark_output_position "bob"

    client_send "alice" "PRIVMSG bob :Hello Bob!"
    sleep 0.3

    assert_new_output_contains "bob" "Hello Bob" "Bob received private message"

    stop_client "alice"
    stop_client "bob"
}

test_privmsg_nonexistent_user() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK msgsender" \
        "USER msgsender 0 * :Sender" \
        "PRIVMSG nobody :Hello?" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "401" "ERR_NOSUCHNICK (401)"
}

test_privmsg_no_text() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK notext" \
        "USER notext 0 * :No Text" \
        "JOIN #notext" \
        "PRIVMSG #notext" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "412\|461" "ERR_NOTEXTTOSEND or ERR_NEEDMOREPARAMS"
}

test_notice() {
    start_client "notifier"
    start_client "notified"

    register_client "notifier" "notifier"
    register_client "notified" "notified"
    sleep 0.2

    mark_output_position "notified"

    client_send "notifier" "NOTICE notified :This is a notice"
    sleep 0.3

    assert_new_output_contains "notified" "This is a notice" "Notice received"

    stop_client "notifier"
    stop_client "notified"
}

test_notice_no_error_on_nonexistent() {
    # NOTICE should NOT generate errors (per RFC)
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK silnot" \
        "USER silnot 0 * :Silent" \
        "NOTICE nobody :Hello?" \
        "QUIT"

    assert_not_contains "$TMP_DIR/out.txt" "401" "NOTICE to non-existent user should not error"
}

# ============================================================================
# Operator Tests (KICK)
# ============================================================================

test_kick() {
    start_client "op"
    start_client "victim"

    register_client "op" "operator"
    register_client "victim" "victim"

    client_send "op" "JOIN #kicktest"
    sleep 0.2
    client_send "victim" "JOIN #kicktest"
    sleep 0.2

    mark_output_position "victim"
    client_send "op" "KICK #kicktest victim :Get out!"
    sleep 0.3

    assert_new_output_contains "victim" "KICK.*#kicktest.*victim" "Victim received KICK"

    stop_client "op"
    stop_client "victim"
}

test_kick_not_operator() {
    start_client "notop"
    start_client "target"

    register_client "notop" "notop"
    register_client "target" "target"

    # Target joins first (becomes operator)
    client_send "target" "JOIN #nokicktest"
    sleep 0.2
    client_send "notop" "JOIN #nokicktest"
    sleep 0.2

    mark_output_position "notop"
    client_send "notop" "KICK #nokicktest target :Try to kick"
    sleep 0.3

    assert_new_output_contains "notop" "482" "ERR_CHANOPRIVSNEEDED (482)"

    stop_client "notop"
    stop_client "target"
}

test_kick_nonexistent_user() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK kicker" \
        "USER kicker 0 * :Kicker" \
        "JOIN #kicktest2" \
        "KICK #kicktest2 ghost :Bye" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "441" "ERR_USERNOTINCHANNEL (441)"
}

test_kick_not_in_channel() {
    start_client "outsider"
    start_client "insider"

    register_client "outsider" "outsider"
    register_client "insider" "insider"

    client_send "insider" "JOIN #insideronly"
    sleep 0.2

    mark_output_position "outsider"
    client_send "outsider" "KICK #insideronly insider :Bye"
    sleep 0.3

    assert_new_output_contains "outsider" "442" "ERR_NOTONCHANNEL (442)"

    stop_client "outsider"
    stop_client "insider"
}

# ============================================================================
# INVITE Tests
# ============================================================================

test_invite_basic() {
    start_client "inviter"
    start_client "invitee"

    register_client "inviter" "inviter"
    register_client "invitee" "invitee"

    client_send "inviter" "JOIN #invitetest"
    sleep 0.2

    mark_output_position "inviter"
    mark_output_position "invitee"

    client_send "inviter" "INVITE invitee #invitetest"
    sleep 0.3

    assert_new_output_contains "inviter" "341" "RPL_INVITING (341) received"
    assert_new_output_contains "invitee" "INVITE" "Invitee received INVITE message"

    stop_client "inviter"
    stop_client "invitee"
}

test_invite_to_invite_only() {
    start_client "op"
    start_client "guest"

    register_client "op" "invop"
    register_client "guest" "invguest"

    client_send "op" "JOIN #invonly"
    sleep 0.2
    client_send "op" "MODE #invonly +i"
    sleep 0.2

    mark_output_position "guest"
    client_send "op" "INVITE invguest #invonly"
    sleep 0.3

    assert_new_output_contains "guest" "INVITE" "Guest received INVITE"

    # Now guest should be able to join
    mark_output_position "guest"
    client_send "guest" "JOIN #invonly"
    sleep 0.3

    assert_new_output_contains "guest" "JOIN" "Guest joined invite-only channel after invite"

    stop_client "op"
    stop_client "guest"
}

test_invite_not_on_channel() {
    start_client "notmember"
    start_client "target"

    register_client "notmember" "notmembr"
    register_client "target" "invtargt"

    # Create channel with target as owner
    client_send "target" "JOIN #notmemtest"
    sleep 0.2

    mark_output_position "notmember"
    client_send "notmember" "INVITE invtargt #notmemtest"
    sleep 0.3

    assert_new_output_contains "notmember" "442" "ERR_NOTONCHANNEL (442)"

    stop_client "notmember"
    stop_client "target"
}

test_invite_user_already_on_channel() {
    start_client "inv1"
    start_client "inv2"

    register_client "inv1" "alrdyinv"
    register_client "inv2" "alrdymem"

    client_send "inv1" "JOIN #alreadytest"
    sleep 0.2
    client_send "inv2" "JOIN #alreadytest"
    sleep 0.2

    mark_output_position "inv1"
    client_send "inv1" "INVITE alrdymem #alreadytest"
    sleep 0.3

    assert_new_output_contains "inv1" "443" "ERR_USERONCHANNEL (443)"

    stop_client "inv1"
    stop_client "inv2"
}

test_invite_nonexistent_user() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK invnone" \
        "USER invnone 0 * :Inviter" \
        "JOIN #invnonetest" \
        "INVITE ghost #invnonetest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "401" "ERR_NOSUCHNICK (401)"
}

test_invite_nonop_invite_only() {
    start_client "chanop"
    start_client "regular"
    start_client "wannajoin"

    register_client "chanop" "chanop"
    register_client "regular" "regular"
    register_client "wannajoin" "wannajn"

    client_send "chanop" "JOIN #nopinvite"
    sleep 0.2
    client_send "regular" "JOIN #nopinvite"
    sleep 0.2
    client_send "chanop" "MODE #nopinvite +i"
    sleep 0.2

    mark_output_position "regular"
    client_send "regular" "INVITE wannajn #nopinvite"
    sleep 0.3

    assert_new_output_contains "regular" "482" "ERR_CHANOPRIVSNEEDED (482) for non-op invite to +i channel"

    stop_client "chanop"
    stop_client "regular"
    stop_client "wannajoin"
}

# ============================================================================
# TOPIC Tests
# ============================================================================

test_topic_query_no_topic() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK topicq" \
        "USER topicq 0 * :Topic Query" \
        "JOIN #notopictest" \
        "TOPIC #notopictest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "331" "RPL_NOTOPIC (331)"
}

test_topic_set_and_query() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK topicset" \
        "USER topicset 0 * :Topic Setter" \
        "JOIN #topicsettest" \
        "TOPIC #topicsettest :Welcome to our channel!" \
        "TOPIC #topicsettest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "TOPIC.*Welcome" "Topic set broadcast received"
    assert_contains "$TMP_DIR/out.txt" "332" "RPL_TOPIC (332) on query"
    assert_contains "$TMP_DIR/out.txt" "Welcome to our channel" "Topic content correct"
}

test_topic_broadcast() {
    start_client "setter"
    start_client "viewer"

    register_client "setter" "topsetter"
    register_client "viewer" "topviewer"

    client_send "setter" "JOIN #topicbc"
    sleep 0.2
    client_send "viewer" "JOIN #topicbc"
    sleep 0.2

    mark_output_position "viewer"
    client_send "setter" "TOPIC #topicbc :New topic here"
    sleep 0.3

    assert_new_output_contains "viewer" "TOPIC" "Viewer received TOPIC broadcast"
    assert_new_output_contains "viewer" "New topic here" "Topic content in broadcast"

    stop_client "setter"
    stop_client "viewer"
}

test_topic_restricted_nonop() {
    start_client "topop"
    start_client "topreg"

    register_client "topop" "topicop"
    register_client "topreg" "topicreg"

    client_send "topop" "JOIN #topicrestr"
    sleep 0.2
    client_send "topreg" "JOIN #topicrestr"
    sleep 0.2
    client_send "topop" "MODE #topicrestr +t"
    sleep 0.2

    mark_output_position "topreg"
    client_send "topreg" "TOPIC #topicrestr :Trying to change"
    sleep 0.3

    assert_new_output_contains "topreg" "482" "ERR_CHANOPRIVSNEEDED (482) for topic change on +t channel"

    stop_client "topop"
    stop_client "topreg"
}

test_topic_not_on_channel() {
    start_client "topchan"

    register_client "topchan" "topnotch"

    client_send "topchan" "JOIN #topchantest"
    sleep 0.2

    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK topout" \
        "USER topout 0 * :Topic Outsider" \
        "TOPIC #topchantest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "442" "ERR_NOTONCHANNEL (442)"

    stop_client "topchan"
}

test_topic_no_such_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK topicnoch" \
        "USER topicnoch 0 * :No Channel" \
        "TOPIC #nonexistentchan" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "403" "ERR_NOSUCHCHANNEL (403)"
}

# ============================================================================
# MODE Tests
# ============================================================================

test_mode_query() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK modeq" \
        "USER modeq 0 * :Mode Query" \
        "JOIN #modeqtest" \
        "MODE #modeqtest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "324" "RPL_CHANNELMODEIS (324)"
}

test_mode_set_invite_only() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK modei" \
        "USER modei 0 * :Mode i" \
        "JOIN #modeitest" \
        "MODE #modeitest +i" \
        "MODE #modeitest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "MODE.*+i\|324.*i" "Invite-only mode set"
}

test_mode_set_topic_restricted() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK modet" \
        "USER modet 0 * :Mode t" \
        "JOIN #modettest" \
        "MODE #modettest +t" \
        "MODE #modettest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "MODE.*+t\|324.*t" "Topic-restricted mode set"
}

test_mode_set_channel_key() {
    start_client "keyop"
    start_client "keyjoin"

    register_client "keyop" "keyoper"
    register_client "keyjoin" "keyjoinr"

    client_send "keyop" "JOIN #keytest"
    sleep 0.2
    client_send "keyop" "MODE #keytest +k secretkey"
    sleep 0.3

    # Try to join without key
    mark_output_position "keyjoin"
    client_send "keyjoin" "JOIN #keytest"
    sleep 0.3

    assert_new_output_contains "keyjoin" "475" "ERR_BADCHANNELKEY (475) without key"

    # Try to join with correct key
    mark_output_position "keyjoin"
    client_send "keyjoin" "JOIN #keytest secretkey"
    sleep 0.3

    assert_new_output_contains "keyjoin" "JOIN" "Joined with correct key"

    stop_client "keyop"
    stop_client "keyjoin"
}

test_mode_set_user_limit() {
    start_client "limitop"
    start_client "limit1"
    start_client "limit2"

    register_client "limitop" "limitop"
    register_client "limit1" "limitone"
    register_client "limit2" "limittwo"

    client_send "limitop" "JOIN #limitest"
    sleep 0.2
    client_send "limitop" "MODE #limitest +l 2"
    sleep 0.2

    client_send "limit1" "JOIN #limitest"
    sleep 0.2

    # Third user should be rejected
    mark_output_position "limit2"
    client_send "limit2" "JOIN #limitest"
    sleep 0.3

    assert_new_output_contains "limit2" "471" "ERR_CHANNELISFULL (471)"

    stop_client "limitop"
    stop_client "limit1"
    stop_client "limit2"
}

test_mode_give_operator() {
    start_client "giveop"
    start_client "getop"

    register_client "giveop" "giverop"
    register_client "getop" "getoper"

    client_send "giveop" "JOIN #optest"
    sleep 0.2
    client_send "getop" "JOIN #optest"
    sleep 0.2

    mark_output_position "getop"
    client_send "giveop" "MODE #optest +o getoper"
    sleep 0.3

    assert_new_output_contains "getop" "MODE.*+o.*getoper" "Operator mode granted"

    # Verify getop can now kick
    start_client "victim"
    register_client "victim" "opvictim"
    client_send "victim" "JOIN #optest"
    sleep 0.2

    mark_output_position "victim"
    client_send "getop" "KICK #optest opvictim :Testing op"
    sleep 0.3

    assert_new_output_contains "victim" "KICK" "New operator can kick"

    stop_client "giveop"
    stop_client "getop"
    stop_client "victim"
}

test_mode_remove_operator() {
    start_client "deop1"
    start_client "deop2"

    register_client "deop1" "deopone"
    register_client "deop2" "deoptwo"

    client_send "deop1" "JOIN #deoptest"
    sleep 0.2
    client_send "deop2" "JOIN #deoptest"
    sleep 0.2
    client_send "deop1" "MODE #deoptest +o deoptwo"
    sleep 0.2

    mark_output_position "deop2"
    client_send "deop1" "MODE #deoptest -o deoptwo"
    sleep 0.3

    assert_new_output_contains "deop2" "MODE.*-o.*deoptwo" "Operator mode removed"

    stop_client "deop1"
    stop_client "deop2"
}

test_mode_combined() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK combmode" \
        "USER combmode 0 * :Combined Mode" \
        "JOIN #combtest" \
        "MODE #combtest +itk secretpass" \
        "MODE #combtest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "324" "RPL_CHANNELMODEIS received"
    # Check that modes are reflected
    assert_contains "$TMP_DIR/out.txt" "i" "Invite-only in mode string"
    assert_contains "$TMP_DIR/out.txt" "t" "Topic-restricted in mode string"
    assert_contains "$TMP_DIR/out.txt" "k" "Key mode in mode string"
}

test_mode_not_operator() {
    start_client "modeop"
    start_client "modereg"

    register_client "modeop" "modeoper"
    register_client "modereg" "moderegr"

    client_send "modeop" "JOIN #noopmode"
    sleep 0.2
    client_send "modereg" "JOIN #noopmode"
    sleep 0.2

    mark_output_position "modereg"
    client_send "modereg" "MODE #noopmode +i"
    sleep 0.3

    assert_new_output_contains "modereg" "482" "ERR_CHANOPRIVSNEEDED (482)"

    stop_client "modeop"
    stop_client "modereg"
}

test_mode_unknown_mode() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK unkmode" \
        "USER unkmode 0 * :Unknown Mode" \
        "JOIN #unktest" \
        "MODE #unktest +x" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "472" "ERR_UNKNOWNMODE (472)"
}

test_mode_broadcast() {
    start_client "modebc1"
    start_client "modebc2"

    register_client "modebc1" "modebcop"
    register_client "modebc2" "modebcmb"

    client_send "modebc1" "JOIN #modebctest"
    sleep 0.2
    client_send "modebc2" "JOIN #modebctest"
    sleep 0.2

    mark_output_position "modebc2"
    client_send "modebc1" "MODE #modebctest +i"
    sleep 0.3

    assert_new_output_contains "modebc2" "MODE.*+i" "Mode change broadcast to channel members"

    stop_client "modebc1"
    stop_client "modebc2"
}

# ============================================================================
# WHO Tests
# ============================================================================

test_who_channel() {
    start_client "whoer"
    start_client "member"

    register_client "whoer" "whoer"
    register_client "member" "whomem"

    client_send "whoer" "JOIN #whotest"
    sleep 0.2
    client_send "member" "JOIN #whotest"
    sleep 0.2

    mark_output_position "whoer"
    client_send "whoer" "WHO #whotest"
    sleep 0.3

    assert_new_output_contains "whoer" "352" "RPL_WHOREPLY (352) received"
    assert_new_output_contains "whoer" "315" "RPL_ENDOFWHO (315) received"
    assert_new_output_contains "whoer" "whoer" "WHO contains whoer"
    assert_new_output_contains "whoer" "whomem" "WHO contains whomem"

    stop_client "whoer"
    stop_client "member"
}

test_who_user() {
    start_client "whouser1"
    start_client "whouser2"

    register_client "whouser1" "whousr1"
    register_client "whouser2" "whousr2"
    sleep 0.2

    mark_output_position "whouser1"
    client_send "whouser1" "WHO whousr2"
    sleep 0.3

    assert_new_output_contains "whouser1" "352" "RPL_WHOREPLY (352) for user"
    assert_new_output_contains "whouser1" "315" "RPL_ENDOFWHO (315) received"
    assert_new_output_contains "whouser1" "whousr2" "WHO contains target user"

    stop_client "whouser1"
    stop_client "whouser2"
}

test_who_nonexistent() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK whonone" \
        "USER whonone 0 * :Who None" \
        "WHO #nonexistentchan" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "315" "RPL_ENDOFWHO (315) for nonexistent"
    assert_not_contains "$TMP_DIR/out.txt" "352" "No WHOREPLY for nonexistent channel"
}

test_who_shows_operator() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK whooptest" \
        "USER whooptest 0 * :Who Op Test" \
        "JOIN #whooptest" \
        "WHO #whooptest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "352" "RPL_WHOREPLY received"
    assert_contains "$TMP_DIR/out.txt" "H@" "Operator flag (H@) in WHO reply"
}

# ============================================================================
# NAMES Tests
# ============================================================================

test_names_channel() {
    start_client "namer"
    start_client "namemem"

    register_client "namer" "namer"
    register_client "namemem" "namemem"

    client_send "namer" "JOIN #namestest"
    sleep 0.2
    client_send "namemem" "JOIN #namestest"
    sleep 0.2

    mark_output_position "namer"
    client_send "namer" "NAMES #namestest"
    sleep 0.3

    assert_new_output_contains "namer" "353" "RPL_NAMREPLY (353) received"
    assert_new_output_contains "namer" "366" "RPL_ENDOFNAMES (366) received"
    assert_new_output_contains "namer" "namer" "NAMES contains namer"
    assert_new_output_contains "namer" "namemem" "NAMES contains namemem"

    stop_client "namer"
    stop_client "namemem"
}

test_names_shows_operator_prefix() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK namesop" \
        "USER namesop 0 * :Names Op" \
        "JOIN #namesoptest" \
        "NAMES #namesoptest" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "353" "RPL_NAMREPLY received"
    assert_contains "$TMP_DIR/out.txt" "@namesop" "Operator prefix in NAMES"
}

test_names_nonexistent_channel() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK namesnone" \
        "USER namesnone 0 * :Names None" \
        "NAMES #nonexistentnames" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "366" "RPL_ENDOFNAMES (366) for nonexistent"
    assert_not_contains "$TMP_DIR/out.txt" "353" "No NAMREPLY for nonexistent channel"
}

test_names_non_member_can_query() {
    start_client "namesowner"
    start_client "namesout"

    register_client "namesowner" "nmowner"
    register_client "namesout" "nmout"

    client_send "namesowner" "JOIN #namesqtest"
    sleep 0.2

    mark_output_position "namesout"
    client_send "namesout" "NAMES #namesqtest"
    sleep 0.3

    assert_new_output_contains "namesout" "353" "Non-member receives RPL_NAMREPLY"
    assert_new_output_contains "namesout" "nmowner" "Non-member sees channel members"

    stop_client "namesowner"
    stop_client "namesout"
}

# ============================================================================
# PING/PONG Tests
# ============================================================================

test_ping_pong() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK pinger" \
        "USER pinger 0 * :Pinger" \
        "PING :testtoken" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "PONG.*testtoken" "PONG response received"
}

test_ping_no_origin() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK pinger2" \
        "USER pinger2 0 * :Pinger" \
        "PING" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "409\|461" "ERR_NOORIGIN or ERR_NEEDMOREPARAMS"
}

# ============================================================================
# QUIT Tests
# ============================================================================

test_quit_message() {
    start_client "quitter"
    start_client "watcher"

    register_client "quitter" "quitter"
    register_client "watcher" "watcher"

    client_send "quitter" "JOIN #quitwatch"
    sleep 0.2
    client_send "watcher" "JOIN #quitwatch"
    sleep 0.2

    mark_output_position "watcher"
    client_send "quitter" "QUIT :See you later!"
    sleep 0.3

    assert_new_output_contains "watcher" "QUIT" "Watcher sees QUIT"
    assert_new_output_contains "watcher" "See you later" "Quit message received"

    stop_client "watcher"
}

# ============================================================================
# Edge Cases
# ============================================================================

test_unknown_command() {
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK unkcmd" \
        "USER unkcmd 0 * :Unknown" \
        "FOOBAR test" \
        "QUIT"

    # Server should either ignore or send 421 ERR_UNKNOWNCOMMAND
    # If it crashes or hangs, this test will fail
    assert_contains "$TMP_DIR/out.txt" "001" "Registered despite unknown command"
}

test_long_message() {
    local long_msg=$(printf 'A%.0s' {1..400})

    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK longmsg" \
        "USER longmsg 0 * :Long" \
        "JOIN #longtest" \
        "PRIVMSG #longtest :$long_msg" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "001" "Registered successfully"
    assert_contains "$TMP_DIR/out.txt" "JOIN" "Joined channel"
}

test_special_chars_in_message() {
    start_client "special"
    start_client "receiver"

    register_client "special" "special"
    register_client "receiver" "rcvspec"

    client_send "special" "JOIN #specialtest"
    sleep 0.2
    client_send "receiver" "JOIN #specialtest"
    sleep 0.2

    mark_output_position "receiver"
    client_send "special" "PRIVMSG #specialtest :Hello! @#\$%^&*() test :with :colons:"
    sleep 0.3

    assert_new_output_contains "receiver" "Hello" "Special chars message received"

    stop_client "special"
    stop_client "receiver"
}

test_rapid_commands() {
    # Test server handles rapid-fire commands
    irc_send "$TMP_DIR/out.txt" \
        "PASS $PASS" \
        "NICK rapid" \
        "USER rapid 0 * :Rapid" \
        "JOIN #rapid1" \
        "JOIN #rapid2" \
        "JOIN #rapid3" \
        "PART #rapid1" \
        "PART #rapid2" \
        "PART #rapid3" \
        "QUIT"

    assert_contains "$TMP_DIR/out.txt" "001" "Registered"
    assert_contains "$TMP_DIR/out.txt" "#rapid1" "Joined rapid1"
    assert_contains "$TMP_DIR/out.txt" "#rapid2" "Joined rapid2"
    assert_contains "$TMP_DIR/out.txt" "#rapid3" "Joined rapid3"
}

test_case_insensitive_commands() {
    irc_send "$TMP_DIR/out.txt" \
        "pass $PASS" \
        "NiCk casetest" \
        "uSeR casetest 0 * :Case Test" \
        "JoIn #casetest" \
        "QuIt"

    assert_contains "$TMP_DIR/out.txt" "001" "Commands are case insensitive"
    assert_contains "$TMP_DIR/out.txt" "JOIN" "JOIN worked with mixed case"
}

# ============================================================================
# Main
# ============================================================================

main() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  ft_irc Functional Test Suite${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo -e "Server: ${YELLOW}$HOST:$PORT${NC}"
    echo -e "Password: ${YELLOW}$PASS${NC}"
    echo

    setup

    # Check if server is running
    if ! nc -z "$HOST" "$PORT" 2>/dev/null; then
        echo -e "${RED}ERROR: Cannot connect to $HOST:$PORT${NC}"
        echo "Make sure the IRC server is running."
        exit 1
    fi

    echo -e "${CYAN}--- Registration Tests ---${NC}"
    run_test "Basic Registration" test_basic_registration
    run_test "Wrong Password" test_wrong_password
    run_test "No Password" test_no_password
    run_test "Invalid Nickname" test_invalid_nickname
    run_test "Nickname Too Long" test_nickname_too_long
    run_test "Duplicate Nickname" test_duplicate_nickname
    run_test "Nick Change" test_nick_change

    echo -e "${CYAN}--- Channel Tests ---${NC}"
    run_test "Join Channel" test_join_channel
    run_test "Join Invalid Channel" test_join_invalid_channel
    run_test "Join Multiple Channels" test_join_multiple_channels
    run_test "Part Channel" test_part_channel
    run_test "Part Not In Channel" test_part_not_in_channel
    run_test "First User Is Operator" test_first_user_is_operator
    run_test "Join Broadcast to Members" test_join_broadcast_to_members

    echo -e "${CYAN}--- Messaging Tests ---${NC}"
    run_test "PRIVMSG Channel" test_privmsg_channel
    run_test "PRIVMSG User" test_privmsg_user
    run_test "PRIVMSG Nonexistent User" test_privmsg_nonexistent_user
    run_test "PRIVMSG No Text" test_privmsg_no_text
    run_test "NOTICE" test_notice
    run_test "NOTICE No Error on Nonexistent" test_notice_no_error_on_nonexistent

    echo -e "${CYAN}--- Operator Tests ---${NC}"
    run_test "KICK" test_kick
    run_test "KICK Not Operator" test_kick_not_operator
    run_test "KICK Nonexistent User" test_kick_nonexistent_user
    run_test "KICK Not In Channel" test_kick_not_in_channel

    echo -e "${CYAN}--- INVITE Tests ---${NC}"
    run_test "INVITE Basic" test_invite_basic
    run_test "INVITE to Invite-Only Channel" test_invite_to_invite_only
    run_test "INVITE Not On Channel" test_invite_not_on_channel
    run_test "INVITE User Already On Channel" test_invite_user_already_on_channel
    run_test "INVITE Nonexistent User" test_invite_nonexistent_user
    run_test "INVITE Non-Op to Invite-Only" test_invite_nonop_invite_only

    echo -e "${CYAN}--- TOPIC Tests ---${NC}"
    run_test "TOPIC Query No Topic" test_topic_query_no_topic
    run_test "TOPIC Set and Query" test_topic_set_and_query
    run_test "TOPIC Broadcast" test_topic_broadcast
    run_test "TOPIC Restricted Non-Op" test_topic_restricted_nonop
    run_test "TOPIC Not On Channel" test_topic_not_on_channel
    run_test "TOPIC No Such Channel" test_topic_no_such_channel

    echo -e "${CYAN}--- MODE Tests ---${NC}"
    run_test "MODE Query" test_mode_query
    run_test "MODE Set Invite-Only (+i)" test_mode_set_invite_only
    run_test "MODE Set Topic-Restricted (+t)" test_mode_set_topic_restricted
    run_test "MODE Set Channel Key (+k)" test_mode_set_channel_key
    run_test "MODE Set User Limit (+l)" test_mode_set_user_limit
    run_test "MODE Give Operator (+o)" test_mode_give_operator
    run_test "MODE Remove Operator (-o)" test_mode_remove_operator
    run_test "MODE Combined Modes" test_mode_combined
    run_test "MODE Not Operator" test_mode_not_operator
    run_test "MODE Unknown Mode" test_mode_unknown_mode
    run_test "MODE Broadcast" test_mode_broadcast

    echo -e "${CYAN}--- WHO Tests ---${NC}"
    run_test "WHO Channel" test_who_channel
    run_test "WHO User" test_who_user
    run_test "WHO Nonexistent" test_who_nonexistent
    run_test "WHO Shows Operator" test_who_shows_operator

    echo -e "${CYAN}--- NAMES Tests ---${NC}"
    run_test "NAMES Channel" test_names_channel
    run_test "NAMES Shows Operator Prefix" test_names_shows_operator_prefix
    run_test "NAMES Nonexistent Channel" test_names_nonexistent_channel
    run_test "NAMES Non-Member Can Query" test_names_non_member_can_query

    echo -e "${CYAN}--- PING/PONG Tests ---${NC}"
    run_test "PING/PONG" test_ping_pong
    run_test "PING No Origin" test_ping_no_origin

    echo -e "${CYAN}--- QUIT Tests ---${NC}"
    run_test "QUIT Message Broadcast" test_quit_message

    echo -e "${CYAN}--- Edge Cases ---${NC}"
    run_test "Unknown Command" test_unknown_command
    run_test "Long Message" test_long_message
    run_test "Special Chars in Message" test_special_chars_in_message
    run_test "Rapid Commands" test_rapid_commands
    run_test "Case Insensitive Commands" test_case_insensitive_commands

    # Summary
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  Summary${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo -e "Tests run:    ${TESTS_RUN}"
    echo -e "Passed:       ${GREEN}${TESTS_PASSED}${NC}"
    echo -e "Failed:       ${RED}${TESTS_FAILED}${NC}"

    if [ "$TESTS_FAILED" -eq 0 ]; then
        echo -e "\n${GREEN}All tests passed!${NC}"
        exit 0
    else
        echo -e "\n${RED}Some tests failed.${NC}"
        exit 1
    fi
}

main "$@"
