NAME = ft_irc

CXX = c++

SRCS_DIR = ./srcs
INC_DIR = ./incs
OBJ_DIR = ./objs



INCLUDES = -I$(INC_DIR) \
			-I$(INC_DIR)/core \
			-I$(INC_DIR)/network

FLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)

SRCS = $(SRCS_DIR)/main.cpp \
		$(SRCS_DIR)/core/Server.cpp \
		$(SRCS_DIR)/core/Client.cpp \
		$(SRCS_DIR)/core/Channel.cpp \
		$(SRCS_DIR)/network/PollSocketManager.cpp \
		$(SRCS_DIR)/network/MessageBuffer.cpp
	   
OBJ = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@) 
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re