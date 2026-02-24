SHELL		:= /bin/bash

NAME		= Railway_Simulator
TEST_NAME	= Railway_Simulator_tests
BUILD_DIR	= build
OUTPUT_DIR	= output
BUILD_TYPE	?= Release

DOCKER_COMPOSE	= docker compose
NETWORK_FILE	?= examples/network_simple.txt
TRAIN_FILE		?= examples/trains_simple.txt
MONTE_RUNS		?= 10
VALGRIND_FLAGS	?= --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1

GREEN	= \033[0;32m
YELLOW	= \033[0;33m
RED		= \033[0;31m
CYAN	= \033[0;36m
RESET	= \033[0m

.PHONY: all build debug release test test-summary test-leaks run valgrind valgrind-test clean fclean re help \
        ensure-output docker-build docker-run docker-monte docker-custom docker-valgrind docker-valgrind-test docker-clean

all: build

define TRAIN
\033[36m
                o    .  o  .  o .  o  .  o  .  o
           o
        .
      .        ___
     _n_n_n____i_i ________ ______________ _++++++++++++++_
  *>(____________I I______I I____________I I______________I
    /ooOOOO OOOOoo  oo oooo oo          oo ooo          ooo
  --------------------------------------------------------------\033[0m
endef
export TRAIN

build:
	@printf "$$TRAIN\n\n"
	@printf "$(YELLOW)  ▶  Configuring...$(RESET)\n"
	@mkdir -p $(BUILD_DIR)
	@cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -S . -B $(BUILD_DIR) 2>&1 \
		|| { printf "$(RED)✖  CMake configuration failed$(RESET)\n"; exit 1; }
	@printf "$(YELLOW)  ▶  Building...$(RESET)\n"
	@set -o pipefail; cmake --build $(BUILD_DIR) -- -j$$(nproc) 2>&1 | \
		while IFS= read -r line; do \
			if echo "$$line" | grep -qE "error:"; then \
				printf "\n$(RED)%s$(RESET)\n" "$$line"; \
			elif echo "$$line" | grep -qE "^\[[ 0-9]+%\]"; then \
				PCT=$$(echo "$$line" | grep -oE "[0-9]+%"); \
				FILE=$$(echo "$$line" | sed "s/.*Building CXX object //;s/\.cpp\.o.*/.cpp/"); \
				printf "\r\033[K$(YELLOW)  ▶  [%s] %s$(RESET)" "$$PCT" "$$FILE"; \
			fi; \
		done; \
		printf "\n"; \
		test "$${PIPESTATUS[0]}" -eq 0 || { printf "$(RED)✖  Build failed$(RESET)\n"; exit 1; }
	@printf "$(GREEN)  ✔  Build complete$(RESET)\n"

debug:
	@$(MAKE) build BUILD_TYPE=Debug

release:
	@$(MAKE) build BUILD_TYPE=Release

test: build
	@printf "$(YELLOW)Running tests...$(RESET)\n\n"
	@./$(BUILD_DIR)/$(TEST_NAME) --gtest_color=yes
	@printf "\n$(GREEN)Tests complete!$(RESET)\n"

test-summary: build
	@printf "$(YELLOW)Running tests (summary mode)...$(RESET)\n"
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@printf "$(GREEN)Tests complete!$(RESET)\n"

test-leaks: debug
	@printf "$(YELLOW)Running tests with valgrind...$(RESET)\n"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--error-exitcode=1 ./$(BUILD_DIR)/$(TEST_NAME)
	@printf "$(GREEN)No memory leaks detected!$(RESET)\n"

run: build
	@printf "$(YELLOW)Running $(NAME)...$(RESET)\n"
	@./$(BUILD_DIR)/$(NAME)

valgrind: debug
	@printf "$(YELLOW)Running valgrind on main executable...$(RESET)\n"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=valgrind-main.log \
		./$(BUILD_DIR)/$(NAME) $(NETWORK_FILE) $(TRAIN_FILE)
	@printf "$(GREEN)Valgrind log saved to valgrind-main.log$(RESET)\n"

valgrind-test: debug
	@printf "$(YELLOW)Running valgrind on tests...$(RESET)\n"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=valgrind-tests.log ./$(BUILD_DIR)/$(TEST_NAME)
	@printf "$(GREEN)Valgrind log saved to valgrind-tests.log$(RESET)\n"

clean:
	@printf "$(YELLOW)Cleaning build artifacts...$(RESET)\n"
	@if [ -d $(BUILD_DIR) ]; then cmake --build $(BUILD_DIR) --target clean > /dev/null 2>&1; fi
	@printf "$(GREEN)Clean complete!$(RESET)\n"

fclean:
	@printf "$(RED)Removing build directory...$(RESET)\n"
	@rm -rf $(BUILD_DIR) $(OUTPUT_DIR) valgrind-main.log valgrind-tests.log
	@printf "$(GREEN)Full clean complete!$(RESET)\n"

re: fclean all

ensure-output:
	@mkdir -p $(OUTPUT_DIR)

docker-build: ensure-output
	@printf "$(YELLOW)Building Docker image...$(RESET)\n"
	@$(DOCKER_COMPOSE) build
	@printf "$(GREEN)Docker image built successfully!$(RESET)\n"

docker-run: ensure-output
	@printf "$(YELLOW)Running simulator inside Docker...$(RESET)\n"
	@$(DOCKER_COMPOSE) run railway $(NETWORK_FILE) $(TRAIN_FILE)

docker-monte: ensure-output
	@printf "$(YELLOW)Running Monte Carlo inside Docker ($(MONTE_RUNS) runs)...$(RESET)\n"
	@$(DOCKER_COMPOSE) run railway $(NETWORK_FILE) $(TRAIN_FILE) --monte-carlo=$(MONTE_RUNS)

docker-custom: ensure-output
	@printf "$(YELLOW)Running custom docker command...$(RESET)\n"
	@$(DOCKER_COMPOSE) run railway $(ARGS)

docker-valgrind: ensure-output
	@printf "$(YELLOW)Running Valgrind inside Docker...$(RESET)\n"
	@$(DOCKER_COMPOSE) run --entrypoint valgrind railway \
		$(VALGRIND_FLAGS) ./build/$(NAME) $(NETWORK_FILE) $(TRAIN_FILE)

docker-valgrind-test: ensure-output
	@printf "$(YELLOW)Running Valgrind on tests inside Docker...$(RESET)\n"
	@$(DOCKER_COMPOSE) run --entrypoint valgrind railway \
		$(VALGRIND_FLAGS) ./build/$(TEST_NAME)

docker-clean:
	@printf "$(RED)Cleaning Docker containers and volumes...$(RESET)\n"
	@docker compose down --remove-orphans || true
	@docker ps -a --filter "name=module05-railway-run" -q | xargs -r docker rm -f
	@docker ps -a --filter "ancestor=module05-railway" -q | xargs -r docker rm -f
	@docker volume rm module05_build_volume || true
	@printf "$(GREEN)Docker cleanup complete!$(RESET)\n"

help:
	@printf "$(GREEN)Railway Simulation — Available targets:$(RESET)\n"
	@printf "  $(YELLOW)make [all]$(RESET)          Build project (default: Release)\n"
	@printf "  $(YELLOW)make debug$(RESET)          Build with Debug flags\n"
	@printf "  $(YELLOW)make release$(RESET)        Build with Release flags\n"
	@printf "  $(YELLOW)make test$(RESET)           Run all tests (detailed output)\n"
	@printf "  $(YELLOW)make test-summary$(RESET)   Run tests (summary only)\n"
	@printf "  $(YELLOW)make test-leaks$(RESET)     Run tests with leak detection\n"
	@printf "  $(YELLOW)make run$(RESET)            Run main executable\n"
	@printf "  $(YELLOW)make valgrind$(RESET)       Run valgrind on main executable\n"
	@printf "  $(YELLOW)make valgrind-test$(RESET)  Run valgrind on tests\n"
	@printf "  $(YELLOW)make clean$(RESET)          Clean build artifacts\n"
	@printf "  $(YELLOW)make fclean$(RESET)         Remove build directory\n"
	@printf "  $(YELLOW)make re$(RESET)             Rebuild from scratch\n"
	@printf "  $(YELLOW)make help$(RESET)           Show this help\n"