#==============================================================================#
#                                   COLORS                                     #
#==============================================================================#
RED			= \033[0;31m
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
MAGENTA		= \033[0;35m
CYAN		= \033[0;36m
WHITE		= \033[0;37m
GRAY        = \033[1;30m
RESET		= \033[0m

#==============================================================================#
#                                   CONFIG                                     #
#==============================================================================#
NAME		= HR_Manager

# Compiler settings
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
DEBUG_FLAGS	= -g -fsanitize=address
INCLUDE		= -I./include -I.

# Source files - auto-detect or specify
SRCS		= $(wildcard *.cpp) $(wildcard src/*.cpp)
OBJS_DIR	= obj
OBJS		= $(SRCS:%.cpp=$(OBJS_DIR)/%.o)

# Create obj directory structure
$(shell mkdir -p $(OBJS_DIR) $(OBJS_DIR)/src)

#==============================================================================#
#                                   RULES                                      #
#==============================================================================#
all:		$(NAME)

$(NAME):	$(OBJS)
			@echo
			@echo "                $(CYAN).7$(RESET)"
			@echo "              $(CYAN).'/$(RESET)"
			@echo "             $(CYAN)/ /$(RESET)"
			@echo "            $(CYAN)/ /$(RESET)"
			@echo "           $(CYAN)/ /$(RESET)"
			@echo "          $(CYAN)/ /$(RESET)"
			@echo "         $(CYAN)/ /$(RESET)"
			@echo "        $(CYAN)/ /$(RESET)"
			@echo "       $(CYAN)/ /$(RESET)"
			@echo "      $(CYAN)/ /$(RESET)"
			@echo "     $(CYAN)/_/$(RESET)"
			@echo "  $(WHITE),-\\__\\"
			@echo "  $(WHITE)|f-\"Y\\|$(RESET)"
			@echo "  $(WHITE)\\()7L/$(RESET)"
			@echo "   $(YELLOW)cgD                           __ _"
			@echo "   $(YELLOW)|\\(                       .'  Y '>,"
			@echo "    $(YELLOW)\\ \\                     / $(RED)_   _$(RESET)   \\"
			@echo "     $(YELLOW)\\ \\                    )$(RED)(_) (_)$(RESET)(|}"
			@echo "      $(YELLOW)\\ \\                  {  4A   } /"
			@echo "       $(YELLOW)\\ \\                  \\uLuJJ/\\l"
			@echo "        $(YELLOW)\\ \\                 |3    p)/"
			@echo "        $(YELLOW)\\ \\ __________      /nnm_n//"
			@echo "        $(YELLOW)c7___-__,__-)\\,__)(\".  \\_>-<_/D"
			@echo "                  $(YELLOW)//V \\_\"-._.__G G_c__.-__<\"/ ( \\"
			@echo "                          <\"-._>__-,G_.___)\\   \\7\\"
			@echo "                         (\"-.__.|\"<.__.-\" )    \\ \\"
			@echo "                         |\"-.__\"\\  |\"-.__.-\".\\  \\ \\"
			@echo "                         (\"-.__\"\". \\-.__.-\".|    \\_\\"
			@echo "                         \\\"-.__\"\"|!|\"-.__.-\".)    \\ \\"
			@echo "                          \"-.__\"\"\\_|\"-.__.-\"./     \\ l"
			@echo "                           \".__\"\"\"\\G>-.__.-\">       .--,_"
			@echo
			@echo "$(BLUE)Linking $(NAME)...$(RESET)"
			@$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJS) -o $(NAME)
			@echo "$(GREEN)✓ Compilation successful! $(NAME) is ready.$(RESET)"
			@echo

$(OBJS_DIR)/%.o: %.cpp
			@mkdir -p $(dir $@)
			@echo "$(CYAN)Compiling: $(RESET)$<"
			@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Debug build with sanitizer
debug:		CXXFLAGS += $(DEBUG_FLAGS)
debug:		re
			@echo "$(MAGENTA)Debug build with AddressSanitizer enabled$(RESET)"

# Clean object files
clean:
			@echo "$(YELLOW)Cleaning object files...$(RESET)"
			@rm -rf $(OBJS_DIR)
			@echo "$(GREEN)✓ Clean done!$(RESET)"

# Clean everything
fclean:		clean
			@echo "$(YELLOW)Cleaning executable...$(RESET)"
			@rm -f $(NAME)
			@echo "$(GREEN)✓ Full clean done!$(RESET)"

# Rebuild
re:			fclean all

# Run the program
run:		all
			@echo "$(CYAN)Running $(NAME)...$(RESET)"
			@./$(NAME)

# Run with valgrind
valgrind:	all
			@echo "$(MAGENTA)Running with valgrind...$(RESET)"
			@valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)

# Show compilation info
info:
			@echo "$(BLUE)Project Information:$(RESET)"
			@echo "  Name:      $(GREEN)$(NAME)$(RESET)"
			@echo "  Compiler:  $(GREEN)$(CXX)$(RESET)"
			@echo "  Flags:     $(GREEN)$(CXXFLAGS)$(RESET)"
			@echo "  Sources:   $(GREEN)$(words $(SRCS)) files$(RESET)"
			@echo "  Objects:   $(GREEN)$(OBJS_DIR)/$(RESET)"
			@echo
			@echo "$(CYAN)Source files:$(RESET)"
			@$(foreach src,$(SRCS),echo "  - $(src)";)

# Help
help:
			@echo "$(CYAN)Available targets:$(RESET)"
			@echo "  $(GREEN)make$(RESET)           - Build the project"
			@echo "  $(GREEN)make debug$(RESET)     - Build with debug flags and sanitizer"
			@echo "  $(GREEN)make clean$(RESET)     - Remove object files"
			@echo "  $(GREEN)make fclean$(RESET)    - Remove objects and executable"
			@echo "  $(GREEN)make re$(RESET)        - Rebuild everything"
			@echo "  $(GREEN)make run$(RESET)       - Build and run the program"
			@echo "  $(GREEN)make valgrind$(RESET)  - Run with valgrind"
			@echo "  $(GREEN)make info$(RESET)      - Show project information"
			@echo "  $(GREEN)make help$(RESET)      - Show this help"
			@echo
			@echo "$(CYAN)Override variables:$(RESET)"
			@echo "  $(GREEN)make NAME=program$(RESET)  - Set custom executable name"

.PHONY:		all clean fclean re debug run valgrind info help