NAME = ft_irc
TEST_NAME = test_runner

CXX = c++

SRCS_DIR = ./srcs
INC_DIR = ./incs
OBJ_DIR = ./objs
TEST_DIR = ./tests
TEST_OBJ_DIR = ./objs/tests



INCLUDES = -I$(INC_DIR) \
			-I$(INC_DIR)/core \
			-I$(INC_DIR)/network

FLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)
TEST_FLAGS = -std=c++11 $(INCLUDES) -lcriterion

SRCS = $(SRCS_DIR)/main.cpp \
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
		$(SRCS_DIR)/protocol/Message.cpp \
		$(SRCS_DIR)/protocol/MessageParser.cpp \
		$(SRCS_DIR)/protocol/NumericReply.cpp \
		$(SRCS_DIR)/protocol/IrcUtils.cpp
	   
OBJ = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_SRCS = $(SRCS_DIR)/protocol/Message.cpp \
			$(SRCS_DIR)/protocol/MessageParser.cpp \
			$(SRCS_DIR)/protocol/NumericReply.cpp \
			$(SRCS_DIR)/protocol/IrcUtils.cpp

TEST_OBJ = $(TEST_SRCS:$(SRCS_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

TEST_FILES = $(TEST_DIR)/test_irc_utils.cpp \
			 $(TEST_DIR)/test_message.cpp \
			 $(TEST_DIR)/test_message_parser.cpp \
			 $(TEST_DIR)/test_numeric_reply.cpp

TEST_FILES_OBJ = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_FLAGS) -c $< -o $@

test: $(TEST_NAME)

$(TEST_NAME): $(TEST_OBJ) $(TEST_FILES_OBJ)
	$(CXX) $(TEST_FLAGS) -o $(TEST_NAME) $(TEST_OBJ) $(TEST_FILES_OBJ)

test_run: $(TEST_NAME)
	./$(TEST_NAME) --verbose

test_filter: $(TEST_NAME)
	./$(TEST_NAME) --filter=$(FILTER) --verbose

docker-test:
	docker run --rm -v $(PWD):/workspace ghcr.io/vantavoids/critervoid:main sh -c "make fclean && make test && make test_run"

docker-test-shell:
	docker run --rm -it -v $(PWD):/workspace ghcr.io/vantavoids/critervoid:main /bin/bash

docker-test-filter:
	docker run --rm -v $(PWD):/workspace ghcr.io/vantavoids/critervoid:main make test_filter FILTER=$(FILTER)

clean:
	rm -f $(OBJ)
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	rm -f $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test test_run test_filter docker-test docker-test-shell docker-test-filter
