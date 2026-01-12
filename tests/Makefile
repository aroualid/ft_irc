TEST_NAME = test_runner

CXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++11
LDFLAGS = -lcriterion

ROOT_DIR = ..
SRCS_DIR = $(ROOT_DIR)/srcs
INC_DIR = $(ROOT_DIR)/incs
TEST_DIR = .
MOCK_DIR = $(TEST_DIR)/mocks
OBJ_DIR = ./objs

INCLUDES = -I$(INC_DIR) \
           -I$(INC_DIR)/core \
           -I$(INC_DIR)/network \
           -I$(INC_DIR)/commands \
           -I$(INC_DIR)/protocol \
           -I$(TEST_DIR)

MOCK_SRCS = $(MOCK_DIR)/Server.cpp \
            $(MOCK_DIR)/Client.cpp \
            $(MOCK_DIR)/Channel.cpp \
            $(MOCK_DIR)/MessageBuffer.cpp

MOCK_OBJ = $(MOCK_SRCS:$(MOCK_DIR)/%.cpp=$(OBJ_DIR)/mocks/%.o)

REQUIRED_SRCS = $(SRCS_DIR)/protocol/Message.cpp \
                $(SRCS_DIR)/protocol/MessageParser.cpp \
                $(SRCS_DIR)/protocol/NumericReply.cpp \
                $(SRCS_DIR)/protocol/IrcUtils.cpp \
                $(SRCS_DIR)/commands/ACommand.cpp \
                $(SRCS_DIR)/commands/JoinCommand.cpp \
                $(SRCS_DIR)/commands/PartCommand.cpp

REQUIRED_OBJ = $(REQUIRED_SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/srcs/%.o)

TEST_SRCS = test_irc_utils.cpp \
            test_message.cpp \
            test_message_parser.cpp \
            test_numeric_reply.cpp \
            test_join_command.cpp \
            test_part_command.cpp

TEST_OBJ = $(TEST_SRCS:%.cpp=$(OBJ_DIR)/%.o)

ALL_OBJ = $(MOCK_OBJ) $(REQUIRED_OBJ) $(TEST_OBJ)

all: $(TEST_NAME)

$(TEST_NAME): $(ALL_OBJ)
	$(CXX) $(FLAGS) -o $(TEST_NAME) $(ALL_OBJ) $(LDFLAGS)
	@echo "Test runner compiled successfully!"

$(OBJ_DIR)/mocks/%.o: $(MOCK_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/srcs/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@

run: $(TEST_NAME)
	./$(TEST_NAME) --verbose

filter: $(TEST_NAME)
	@if [ -z "$(FILTER)" ]; then \
		echo "Usage: make filter FILTER=<test_name>"; \
		exit 1; \
	fi
	./$(TEST_NAME) --filter=$(FILTER) --verbose

docker-test:
	docker run --rm -t --user $(shell id -u):$(shell id -g) -v $(CURDIR)/..:/workspace -w /workspace/tests \
		ghcr.io/vantavoids/critervoid:main sh -c "make fclean && make && make run"

docker-test-shell:
	docker run --rm -t --user $(shell id -u):$(shell id -g) -it -v $(CURDIR)/..:/workspace -w /workspace/tests \
		ghcr.io/vantavoids/critervoid:main /bin/bash

docker-test-filter:
	@if [ -z "$(FILTER)" ]; then \
		echo "Usage: make docker-test-filter FILTER=<test_name>"; \
		exit 1; \
	fi
	docker run --rm -t --user $(shell id -u):$(shell id -g) -v $(CURDIR)/..:/workspace -w /workspace/tests \
		ghcr.io/vantavoids/critervoid:main make filter FILTER=$(FILTER)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TEST_NAME)

re: fclean all

.PHONY: all run filter docker-test docker-test-shell docker-test-filter clean fclean re
