# Railway Simulation - Makefile Orchestrator

NAME		= Railway_Simulator
TEST_NAME	= Railway_Simulator_tests
BUILD_DIR	= build
OUTPUT_DIR	= output
BUILD_TYPE	?= Release

# Docker configuration
DOCKER_COMPOSE = docker compose
NETWORK_FILE ?= examples/network_simple.txt
TRAIN_FILE ?= examples/trains_simple.txt
MONTE_RUNS ?= 10
VALGRIND_FLAGS ?= --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1

# Colors
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
RED			= \033[0;31m
RESET		= \033[0m

.PHONY: all build test test-summary test-leaks clean fclean re run valgrind valgrind-test help debug release

all: build

# Create build directory and compile
build:
	@echo "$(YELLOW)Creating build directory...$(RESET)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ..
	@echo "$(YELLOW)Building project...$(RESET)"
	@cmake --build $(BUILD_DIR) -- -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
	@echo "$(GREEN)Build complete!$(RESET)"

# Debug build
debug:
	@$(MAKE) build BUILD_TYPE=Debug

# Release build
release:
	@$(MAKE) build BUILD_TYPE=Release

# Run tests with detailed output
test: build
	@echo "$(YELLOW)Running tests...$(RESET)"
	@echo ""
	@./$(BUILD_DIR)/$(TEST_NAME) --gtest_color=yes
	@echo ""
	@echo "$(GREEN)Tests complete!$(RESET)"

# Run tests with ctest (simple summary)
test-summary: build
	@echo "$(YELLOW)Running tests (summary mode)...$(RESET)"
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo "$(GREEN)Tests complete!$(RESET)"

# Run tests with memory leak detection
test-leaks: debug
	@echo "$(YELLOW)Running tests with valgrind (leak detection)...$(RESET)"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--error-exitcode=1 \
		./$(BUILD_DIR)/$(TEST_NAME)
	@echo "$(GREEN)No memory leaks detected!$(RESET)"

# Run main executable
run: build
	@echo "$(YELLOW)Running $(NAME)...$(RESET)"
	@./$(BUILD_DIR)/$(NAME)

# Run with valgrind (memory leak detection)
valgrind: debug
	@echo "$(YELLOW)Running valgrind on main executable...$(RESET)"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=valgrind-main.log \
		./$(BUILD_DIR)/$(NAME)
	@echo "$(GREEN)Valgrind log saved to valgrind-main.log$(RESET)"

# Run valgrind on tests
valgrind-test: debug
	@echo "$(YELLOW)Running valgrind on tests...$(RESET)"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=valgrind-tests.log \
		./$(BUILD_DIR)/$(TEST_NAME)
	@echo "$(GREEN)Valgrind log saved to valgrind-tests.log$(RESET)"

# Clean build artifacts (keep build directory)
clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(RESET)"
	@if [ -d $(BUILD_DIR) ]; then cmake --build $(BUILD_DIR) --target clean; fi
	@echo "$(GREEN)Clean complete!$(RESET)"

# Full clean (remove build directory)
fclean:
	@echo "$(RED)Removing build directory...$(RESET)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(OUTPUT_DIR)
	@rm -f valgrind-main.log valgrind-tests.log
	@echo "$(GREEN)Full clean complete!$(RESET)"

# Rebuild from scratch
re: fclean all

# # Install executable to system (requires sudo)
# install: release
# 	@echo "$(YELLOW)Installing $(NAME)...$(RESET)"
# 	@sudo cmake --install $(BUILD_DIR)
# 	@echo "$(GREEN)Installation complete!$(RESET)"

# ------------------ DOCKER TARGETS ------------------

# Ensure output directory exists before any docker operation
ensure-output:
	@echo "$(YELLOW)Checking output directory...$(RESET)"
	@mkdir -p $(OUTPUT_DIR)

# Build docker image using compose
docker-build:
	@echo "$(YELLOW)Building Docker image...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) build
	@echo "$(GREEN)Docker image built successfully!$(RESET)"

# Run simulator inside docker
docker-run:
	@echo "$(YELLOW)Running simulator inside Docker...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) run railway $(NETWORK_FILE) $(TRAIN_FILE)

# Run Monte Carlo simulation inside docker
docker-monte:
	@echo "$(YELLOW)Running Monte Carlo inside Docker ($(MONTE_RUNS) runs)...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) run railway $(NETWORK_FILE) $(TRAIN_FILE) --monte-carlo=$(MONTE_RUNS)

# Run with custom arguments
docker-custom:
	@echo "$(YELLOW)Running custom docker command...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) run railway $(ARGS)

# Run simulator under valgrind inside docker (overrides ENTRYPOINT)
docker-valgrind:
	@echo "$(YELLOW)Running Valgrind inside Docker...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) run --entrypoint valgrind railway \
		$(VALGRIND_FLAGS) \
		./build/$(NAME) $(NETWORK_FILE) $(TRAIN_FILE)

# Run unit tests under valgrind inside docker (overrides ENTRYPOINT)
docker-valgrind-test:
	@echo "$(YELLOW)Running Valgrind on tests inside Docker...$(RESET)"
	@$(MAKE) ensure-output
	@$(DOCKER_COMPOSE) run --entrypoint valgrind railway \
		$(VALGRIND_FLAGS) \
		./build/$(TEST_NAME)

# Clean docker containers and volumes
docker-clean:
	@echo "$(RED)Cleaning Docker containers and volumes...$(RESET)"

	@echo "$(YELLOW)Stopping any running compose containers...$(RESET)"
	@docker compose down --remove-orphans || true

	@echo "$(YELLOW)Removing old run containers...$(RESET)"
	@docker ps -a --filter "name=module05-railway-run" -q | xargs -r docker rm -f

	@echo "$(YELLOW)Removing dangling containers from this image...$(RESET)"
	@docker ps -a --filter "ancestor=module05-railway" -q | xargs -r docker rm -f

	@echo "$(YELLOW)Removing build volume...$(RESET)"
	@docker volume rm module05_build_volume || true

	@echo "$(GREEN)Docker cleanup complete!$(RESET)"

# Show help
help:
	@echo "$(GREEN)Railway Simulation - Available targets:$(RESET)"
	@echo "  $(YELLOW)make [all]$(RESET)        - Build project (default: Release)"
	@echo "  $(YELLOW)make build$(RESET)        - Build project"
	@echo "  $(YELLOW)make debug$(RESET)        - Build with Debug flags"
	@echo "  $(YELLOW)make release$(RESET)      - Build with Release flags"
	@echo "  $(YELLOW)make test$(RESET)         - Run all tests (detailed output)"
	@echo "  $(YELLOW)make test-summary$(RESET) - Run tests (summary only)"
	@echo "  $(YELLOW)make test-leaks$(RESET)   - Run tests with leak detection"
	@echo "  $(YELLOW)make run$(RESET)          - Run main executable"
	@echo "  $(YELLOW)make valgrind$(RESET)     - Run valgrind on main executable"
	@echo "  $(YELLOW)make valgrind-test$(RESET) - Run valgrind on tests"
	@echo "  $(YELLOW)make clean$(RESET)        - Clean build artifacts"
	@echo "  $(YELLOW)make fclean$(RESET)       - Remove build directory"
	@echo "  $(YELLOW)make re$(RESET)           - Rebuild from scratch"
	@echo "  $(YELLOW)make install$(RESET)      - Install to system (requires sudo)"
	@echo "  $(YELLOW)make help$(RESET)         - Show this help message"
	@echo ""
	@echo "$(GREEN)Examples:$(RESET)"
	@echo "  make test          - Run tests with detailed output"
	@echo "  make test-leaks    - Check tests for memory leaks"
	@echo "  make debug test    - Build debug version and run tests"
	@echo "  make fclean all    - Clean rebuild"