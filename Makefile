# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: elizasikira <elizasikira@student.42.fr>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/11/29 15:50:50 by elsikira          #+#    #+#              #
#    Updated: 2025/12/02 21:43:29 by elizasikira      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_irc

CXX = c++

FLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INC_DIR)

SRCS_DIR = ./srcs
INC_DIR = ./incs
OBJ_DIR = ./objs

SRCS = main.cpp
	   
OBJ = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re