INCLUDE			= include
NAME			= proxy_server
SRC				= main.cpp \
				  Server.cpp \
				  Client.cpp

SRCDIR			= src
OBJDIR			= obj
OBJ				= $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))
CC				= clang++
CFLAGS			= -Wall -Wextra -Werror

$(NAME):		$(OBJ)
				$(CC) $(OBJ) -o $(NAME) -g

$(OBJDIR):
				mkdir -p $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp | $(OBJDIR)
				$(CC) $(CFLAGS) -std=c++11 -I$(INCLUDE) -c -MD $< -o $@ -g

include $(wildcard $(OBJDIR)/*.d)

all:			$(NAME)

clean:
				rm -rf $(OBJDIR)

fclean:			clean
				rm -rf $(NAME) test

re:				fclean all

.PHONY:			all clean fclean re
