NAME := ircserv
ANNOUNCER_NAME := Announcer

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP

SRCS_DIR := ./srcs
INC_DIR := ./incs
OBJ_DIR := ./objs
TEST_DIR := ./tests

INCLUDES := -I$(INC_DIR) \
			-I$(INC_DIR)/core \
			-I$(INC_DIR)/network \
			-I$(INC_DIR)/commands \
			-I$(INC_DIR)/bot

BONUS_INCLUDES := -I$(INC_DIR)/bot

get_color = $(if $(filter Purple,$(1)),$(shell tput setaf 5),$(if $(filter Red,$(1)),$(shell tput setaf 1),$(if $(filter Cyan,$(1)),$(shell tput setaf 6),$(if $(filter Blue,$(1)),$(shell tput setaf 4),$(if $(filter Yellow,$(1)),$(shell tput setaf 3),$(if $(filter Green,$(1)),$(shell tput setaf 2),$(if $(filter Off,$(1)),$(shell tput sgr0),$(shell tput sgr0))))))))

ifdef DEBUG
	CXXFLAGS += -g3 -DDEBUG_MODE
else ifdef FSAN
	CXXFLAGS += -g3 -fsanitize=address -DDEBUG_MODE
endif

SRCS := $(SRCS_DIR)/main.cpp \
		$(SRCS_DIR)/core/Config.cpp \
		$(SRCS_DIR)/core/Server.cpp \
		$(SRCS_DIR)/core/Client.cpp \
		$(SRCS_DIR)/core/Channel.cpp \
		$(SRCS_DIR)/core/Logger.cpp \
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
		$(SRCS_DIR)/commands/KickCommand.cpp \
		$(SRCS_DIR)/commands/InviteCommand.cpp \
		$(SRCS_DIR)/commands/TopicCommand.cpp \
		$(SRCS_DIR)/commands/ModeCommand.cpp \
		$(SRCS_DIR)/commands/WhoCommand.cpp \
		$(SRCS_DIR)/commands/NamesCommand.cpp \
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

BONUS_SRCS := $(SRCS_DIR)/bot/BotMessageBuffer.cpp \
			  $(SRCS_DIR)/bot/BotClient.cpp \
			  $(SRCS_DIR)/bot/Miaou.cpp \
			  $(SRCS_DIR)/bot/SixSevenBot.cpp

OBJS := $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS_BONUS := $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/bonus/%.o) \
			  $(BONUS_SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/bonus/%.o)

BONUS_MARKER := .bonus

.PHONY: all bonus clean fclean re debug fsan format lint lint-fix \
		test test_run test_filter docker-test docker-test-shell docker-test-filter

all:
	@if $(MAKE) --no-print-directory -q $(NAME) 2>/dev/null; then \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Everything is up to date. $(call get_color,Purple)Zzz...$(call get_color,Off)"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Starting build of $(call get_color,Purple)$(NAME)$(call get_color,Off)..."; \
		$(MAKE) --no-print-directory $(NAME); \
	fi

bonus:
	@if [ -f $(BONUS_MARKER) ] && $(MAKE) --no-print-directory -q $(BONUS_MARKER) 2>/dev/null; then \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Bonus is already up to date. $(call get_color,Purple)Zzz...$(call get_color,Off)"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Starting build of $(call get_color,Yellow)$(NAME)$(call get_color,Off) (with bots. many of these.)..."; \
		$(MAKE) --no-print-directory $(BONUS_MARKER); \
	fi

-include $(OBJS:.o=.d)
-include $(OBJS_BONUS:.o=.d)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Cyan)Compiling $<$(call get_color,Off)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ || \
		(echo "\n$(call get_color,Red)Compilation failed for $<$(call get_color,Off)" && exit 1)

$(OBJ_DIR)/bonus/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Cyan)Compiling $<$(call get_color,Off)"
	@$(CXX) $(CXXFLAGS) -DBONUS $(INCLUDES) $(BONUS_INCLUDES) -c $< -o $@ || \
		(echo "\n$(call get_color,Red)Compilation failed for $<$(call get_color,Off)" && exit 1)

$(NAME): $(OBJS)
	@printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Green)Linking $(NAME)$(call get_color,Off)\n"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(NAME) $(OBJS) || \
		(echo "$(call get_color,Red)Linking failed!$(call get_color,Off)" && exit 1)
	@if [ "$(DEBUG)" = "1" ]; then \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) compiled in $(call get_color,Red)DEBUG$(call get_color,Off) mode!"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) has been compiled! Netsplits are now a philosophical concept and PING timeouts build character :) (discord remains the disaster recovery plan)"; \
	fi

$(BONUS_MARKER): $(OBJS_BONUS)
	@printf "\r\033[K$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Green)Linking $(NAME) with bonus$(call get_color,Off)\n"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(BONUS_INCLUDES) -o $(NAME) $(OBJS_BONUS) || \
		(echo "$(call get_color,Red)Linking failed!$(call get_color,Off)" && exit 1)
	@touch $(BONUS_MARKER)
	@if [ "$(DEBUG)" = "1" ]; then \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) with $(call get_color,Yellow)BONUS$(call get_color,Off) compiled in $(call get_color,Red)DEBUG$(call get_color,Off) mode!"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) with $(call get_color,Yellow)BONUS$(call get_color,Off) has been compiled!"; \
	fi

clean:
	@echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Cleaning object files..."
	@if [ -d $(OBJ_DIR) ]; then \
		rm -rf $(OBJ_DIR) && \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Removed object and dependency files"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] No object files to clean"; \
	fi
	@if [ -f $(BONUS_MARKER) ]; then \
		rm -f $(BONUS_MARKER) && \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Removed bonus marker"; \
	fi
	@$(MAKE) --no-print-directory -C $(TEST_DIR) clean

fclean: clean
	@if [ -f $(NAME) ]; then \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] Removing $(call get_color,Purple)$(NAME)$(call get_color,Off)..."; \
		rm -f $(NAME) && \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) is GONE!!"; \
	else \
		echo "$(call get_color,Off)[$(ANNOUNCER_NAME)] $(call get_color,Purple)$(NAME)$(call get_color,Off) is already gone"; \
	fi
	@$(MAKE) --no-print-directory -C $(TEST_DIR) fclean

re: fclean all

debug:
	@$(MAKE) --no-print-directory re DEBUG=1

fsan:
	@$(MAKE) --no-print-directory re FSAN=1

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

format:
	@find . -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +

lint:
	run-clang-tidy

lint-fix:
	run-clang-tidy -fix
