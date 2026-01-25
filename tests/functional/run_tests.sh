#!/bin/bash
# Functional test suite for ft_irc using netcat
# Usage: ./run_tests.sh [host] [port] [password]
#
# Tested commands: PASS, NICK, USER, JOIN, PART, PRIVMSG, NOTICE, KICK, PING, QUIT
# Not yet implemented: MODE, TOPIC, INVITE

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
