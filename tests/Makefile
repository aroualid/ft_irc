MAKEFLAGS += --no-print-directory

TEST_NAME = test_runner
ANNOUNCER = TestRunner

CXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++11
LDFLAGS = -lcriterion

get_color = $(if $(filter Purple,$(1)),$(shell tput setaf 5),$(if $(filter Red,$(1)),$(shell tput setaf 1),$(if $(filter Cyan,$(1)),$(shell tput setaf 6),$(if $(filter Blue,$(1)),$(shell tput setaf 4),$(if $(filter Yellow,$(1)),$(shell tput setaf 3),$(if $(filter Green,$(1)),$(shell tput setaf 2),$(if $(filter Off,$(1)),$(shell tput sgr0),$(shell tput sgr0))))))))

total_files = $(words $(1))
current_file := 0

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
                $(SRCS_DIR)/commands/PartCommand.cpp \
                $(SRCS_DIR)/commands/PrivmsgCommand.cpp \
                $(SRCS_DIR)/commands/NoticeCommand.cpp

REQUIRED_OBJ = $(REQUIRED_SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/srcs/%.o)

TEST_SRCS = test_irc_utils.cpp \
            test_message.cpp \
            test_message_parser.cpp \
            test_numeric_reply.cpp \
            test_join_command.cpp \
            test_part_command.cpp \
            test_privmsg_command.cpp \
            test_notice_command.cpp

TEST_OBJ = $(TEST_SRCS:%.cpp=$(OBJ_DIR)/%.o)

ALL_OBJ = $(MOCK_OBJ) $(REQUIRED_OBJ) $(TEST_OBJ)
ALL_SRCS = $(MOCK_SRCS) $(REQUIRED_SRCS) $(TEST_SRCS)

all:
	@if $(MAKE) -q $(TEST_NAME) 2>/dev/null; then \
		printf "$(call get_color,Off)[$(ANNOUNCER)] Everything is up to date $(call get_color,Green)✓$(call get_color,Off)\n"; \
	else \
		printf "$(call get_color,Off)[$(ANNOUNCER)] Building $(call get_color,Cyan)$(TEST_NAME)$(call get_color,Off)...\n"; \
		$(MAKE) $(TEST_NAME) TOTAL_FILES=$(call total_files,$(ALL_SRCS)); \
	fi

$(TEST_NAME): $(ALL_OBJ)
	@printf "$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Green)Linking $(TEST_NAME)$(call get_color,Off)\n"
	@$(CXX) $(FLAGS) -o $(TEST_NAME) $(ALL_OBJ) $(LDFLAGS) || \
		(printf "$(call get_color,Red)[$(ANNOUNCER)] Linking failed!$(call get_color,Off)\n" && exit 1)
	@printf "$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Cyan)$(TEST_NAME)$(call get_color,Off) compiled successfully $(call get_color,Green)✓$(call get_color,Off)\n"

$(OBJ_DIR)/mocks/%.o: $(MOCK_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@INDEX=$$(find $(OBJ_DIR) -name "*.o" 2>/dev/null | wc -l); \
		printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Cyan)Compiling $$((INDEX + 1))/$(TOTAL_FILES)$(call get_color,Off)"; \
		$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@ || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Compilation failed for $<$(call get_color,Off)\n" && exit 1)
	@printf "\n"

$(OBJ_DIR)/srcs/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@INDEX=$$(find $(OBJ_DIR) -name "*.o" 2>/dev/null | wc -l); \
		printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Cyan)Compiling $$((INDEX + 1))/$(TOTAL_FILES)$(call get_color,Off)"; \
		$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@ || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Compilation failed for $<$(call get_color,Off)\n" && exit 1)
	@printf "\n"

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@INDEX=$$(find $(OBJ_DIR) -name "*.o" 2>/dev/null | wc -l); \
		printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Cyan)Compiling $$((INDEX + 1))/$(TOTAL_FILES)$(call get_color,Off)"; \
		$(CXX) $(FLAGS) $(INCLUDES) -c $< -o $@ || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Compilation failed for $<$(call get_color,Off)\n" && exit 1)
	@printf "\n"

run: $(TEST_NAME)
	./$(TEST_NAME) --verbose

filter: $(TEST_NAME)
	@if [ -z "$(FILTER)" ]; then \
		echo "Usage: make filter FILTER=<test_name>"; \
		exit 1; \
	fi
	./$(TEST_NAME) --filter=$(FILTER) --verbose

DOCKER_IMAGE = ghcr.io/vantavoids/critervoid:main
DOCKER_VOLUME = -v $(CURDIR)/..:/workspace
DOCKER_WORKDIR = -w /workspace/tests

CURRENT_UID := $(shell id -u)
CURRENT_GID := $(shell id -g)
ifeq ($(shell [ $(CURRENT_UID) -lt 65536 ] && echo yes || echo no),yes)
	DOCKER_USER_FLAG = --user $(CURRENT_UID):$(CURRENT_GID)
else
	DOCKER_USER_FLAG = --user 0:0
endif

DOCKER_RUN = docker run --rm -t $(DOCKER_USER_FLAG) $(DOCKER_VOLUME) $(DOCKER_WORKDIR) $(DOCKER_IMAGE)
DOCKER_RUN_IT = docker run --rm -it $(DOCKER_USER_FLAG) $(DOCKER_VOLUME) $(DOCKER_WORKDIR) $(DOCKER_IMAGE)

docker-pull:
	@printf "$(call get_color,Off)[$(ANNOUNCER)] Pulling latest $(call get_color,Purple)critervoid$(call get_color,Off) image...\n"
	@docker pull $(DOCKER_IMAGE)

docker-test:
	@printf "$(call get_color,Off)[$(ANNOUNCER)] Running all tests in docker $(call get_color,Blue)(clean build)$(call get_color,Off)...\n"
	@$(DOCKER_RUN) sh -c "make fclean && make && make run" || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Tests failed!$(call get_color,Off) Run '$(call get_color,Yellow)make docker-test-shell$(call get_color,Off)' to debug.\n" && exit 1)

docker-test-quick:
	@printf "$(call get_color,Off)[$(ANNOUNCER)] Running tests in docker $(call get_color,Yellow)(incremental)$(call get_color,Off)...\n"
	@$(DOCKER_RUN) sh -c "make && make run" || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Tests failed!$(call get_color,Off) Try '$(call get_color,Yellow)make docker-test$(call get_color,Off)' for a clean build.\n" && exit 1)

docker-test-shell:
	@printf "$(call get_color,Off)[$(ANNOUNCER)] Starting interactive shell in docker...\n"
	@printf "$(call get_color,Cyan)Tip:$(call get_color,Off) Run '$(call get_color,Yellow)make && make run$(call get_color,Off)' inside the container to build and test.\n"
	@$(DOCKER_RUN_IT) /bin/bash || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Cannot start interactive shell.$(call get_color,Off) Make sure you have a TTY.\n" && exit 1)

docker-test-filter:
	@if [ -z "$(FILTER)" ]; then \
		printf "$(call get_color,Red)[$(ANNOUNCER)] Missing FILTER parameter$(call get_color,Off)\n"; \
		printf "Usage: $(call get_color,Yellow)make docker-test-filter FILTER=<pattern>$(call get_color,Off)\n"; \
		printf "Example: $(call get_color,Cyan)make docker-test-filter FILTER='IrcUtils/*'$(call get_color,Off)\n"; \
		exit 1; \
	fi
	@printf "$(call get_color,Off)[$(ANNOUNCER)] Running filtered tests: $(call get_color,Purple)$(FILTER)$(call get_color,Off)...\n"
	@$(DOCKER_RUN) sh -c "make fclean && make && ./test_runner --filter='$(FILTER)' --verbose" || \
		(printf "\n$(call get_color,Red)[$(ANNOUNCER)] Filtered tests failed!$(call get_color,Off)\n" && exit 1)

clean:
	@if [ -d $(OBJ_DIR) ]; then \
		rm -rf $(OBJ_DIR) && \
		printf "$(call get_color,Off)[$(ANNOUNCER)] Cleaned object files $(call get_color,Green)✓$(call get_color,Off)\n"; \
	else \
		printf "$(call get_color,Off)[$(ANNOUNCER)] Nothing to clean\n"; \
	fi

fclean: clean
	@if [ -f $(TEST_NAME) ]; then \
		rm -f $(TEST_NAME) && \
		printf "$(call get_color,Off)[$(ANNOUNCER)] Removed $(call get_color,Cyan)$(TEST_NAME)$(call get_color,Off) $(call get_color,Green)✓$(call get_color,Off)\n"; \
	else \
		printf "$(call get_color,Off)[$(ANNOUNCER)] $(call get_color,Cyan)$(TEST_NAME)$(call get_color,Off) already removed\n"; \
	fi

re: fclean all

.PHONY: all run filter docker-pull docker-test docker-test-quick docker-test-shell docker-test-filter clean fclean re
