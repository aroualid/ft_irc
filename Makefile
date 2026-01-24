NAME = ircserv

CXX = c++

SRCS_DIR = ./srcs
INC_DIR = ./incs
OBJ_DIR = ./objs
TEST_DIR = ./tests


INCLUDES = -I$(INC_DIR) \
			-I$(INC_DIR)/core \
			-I$(INC_DIR)/network \
			-I$(INC_DIR)/commands \

FLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)

SRCS = $(SRCS_DIR)/main.cpp \
		$(SRCS_DIR)/core/Config.cpp \
		$(SRCS_DIR)/core/Server.cpp \
		$(SRCS_DIR)/core/Client.cpp \
		$(SRCS_DIR)/core/Channel.cpp \
		$(SRCS_DIR)/network/PollSocketManager.cpp \
		$(SRCS_DIR)/network/MessageBuffer.cpp \
		$(SRCS_DIR)/commands/ACommand.cpp \
		$(SRCS_DIR)/commands/CommandFactory.cpp \
		$(SRCS_DIR)/commands/PassCommand.cpp \
		$(SRCS_DIR)/commands/NickCommand.cpp \
		$(SRCS_DIR)/commands/UserCommand.cpp \
		$(SRCS_DIR)/commands/JoinCommand.cpp \
		$(SRCS_DIR)/commands/PartCommand.cpp \
		$(SRCS_DIR)/commands/PrivmsgCommand.cpp \
		$(SRCS_DIR)/commands/NoticeCommand.cpp \
		$(SRCS_DIR)/commands/QuitCommand.cpp \
		$(SRCS_DIR)/commands/PingCommand.cpp \
		$(SRCS_DIR)/commands/PongCommand.cpp \
		$(SRCS_DIR)/protocol/Message.cpp \
		$(SRCS_DIR)/protocol/MessageParser.cpp \
		$(SRCS_DIR)/protocol/NumericReply.cpp \
		$(SRCS_DIR)/modes/InviteOnlyMode.cpp \
		$(SRCS_DIR)/modes/TopicRestrictedMode.cpp \
		$(SRCS_DIR)/modes/OperatorMode.cpp \
		$(SRCS_DIR)/modes/KeyMode.cpp \
		$(SRCS_DIR)/modes/UserLimitMode.cpp \
		$(SRCS_DIR)/protocol/IrcUtils.cpp
	   
OBJ = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@) 
	$(CXX) $(FLAGS) -c $< -o $@

test:
	@$(MAKE) -C $(TEST_DIR)

test_run:
	@$(MAKE) -C $(TEST_DIR) run

test_filter:
	@$(MAKE) -C $(TEST_DIR) filter FILTER=$(FILTER)

docker-test:
	@$(MAKE) -C $(TEST_DIR) docker-test

docker-test-shell:
	@$(MAKE) -C $(TEST_DIR) docker-test-shell

docker-test-filter:
	@$(MAKE) -C $(TEST_DIR) docker-test-filter FILTER=$(FILTER)

clean:
	rm -f $(OBJ)
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(TEST_DIR) clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C $(TEST_DIR) fclean

re: fclean all

format:
	@find . -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +

lint:
	run-clang-tidy

lint-fix:
	run-clang-tidy -fix

.PHONY: all clean fclean re format lint lint-fix test test_run test_filter docker-test docker-test-shell docker-test-filter
