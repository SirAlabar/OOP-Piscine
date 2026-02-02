#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
RESET='\033[0m'

EXECUTABLE="./build/Railway_Simulator"

echo -e "${BLUE}========================================${RESET}"
echo -e "${BLUE}  Railway Simulation - Example Tests${RESET}"
echo -e "${BLUE}========================================${RESET}"
echo ""

# Build project
echo -e "${YELLOW}Building project...${RESET}"
make fclean > /dev/null 2>&1
make build > /dev/null 2>&1

if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Build failed!${RESET}"
    exit 1
fi

echo -e "${GREEN}Build successful!${RESET}"
echo ""

# Test counter
PASSED=0
FAILED=0

# Helper function to run test
run_test() {
    local test_name=$1
    local network=$2
    local trains=$3
    local should_pass=$4

    echo -e "${YELLOW}Test: ${test_name}${RESET}"
    
    $EXECUTABLE "$network" "$trains" > /tmp/test_output.txt 2>&1
    exit_code=$?

    if [ $should_pass -eq 1 ]; then
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}  ✓ PASSED${RESET}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}  ✗ FAILED (expected success, got error)${RESET}"
            cat /tmp/test_output.txt | head -5
            FAILED=$((FAILED + 1))
        fi
    else
        if [ $exit_code -ne 0 ]; then
            echo -e "${GREEN}  ✓ PASSED (correctly rejected invalid input)${RESET}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}  ✗ FAILED (should have failed but passed)${RESET}"
            FAILED=$((FAILED + 1))
        fi
    fi
    echo ""
}

# Valid tests
echo -e "${BLUE}=== Valid Input Tests ===${RESET}"
echo ""

run_test "Simple network with simple trains" \
    "examples/network_simple.txt" \
    "examples/trains_simple.txt" \
    1

run_test "Complex network with simple trains" \
    "examples/network_complex.txt" \
    "examples/trains_simple.txt" \
    1

# Invalid tests
echo -e "${BLUE}=== Invalid Input Tests ===${RESET}"
echo ""

run_test "Duplicate node names" \
    "examples/network_invalid_duplicate.txt" \
    "examples/trains_simple.txt" \
    0

run_test "Non-existent node in rail" \
    "examples/network_invalid_rail.txt" \
    "examples/trains_simple.txt" \
    0

run_test "Negative rail length" \
    "examples/network_invalid_negative.txt" \
    "examples/trains_simple.txt" \
    0

run_test "Zero train mass" \
    "examples/network_simple.txt" \
    "examples/trains_invalid_mass.txt" \
    0

run_test "Invalid time format" \
    "examples/network_simple.txt" \
    "examples/trains_invalid_time.txt" \
    0

run_test "Missing train fields" \
    "examples/network_simple.txt" \
    "examples/trains_invalid_fields.txt" \
    0

# CLI tests
echo -e "${BLUE}=== CLI Tests ===${RESET}"
echo ""

echo -e "${YELLOW}Test: Help flag${RESET}"
$EXECUTABLE --help > /tmp/test_output.txt 2>&1
if [ $? -eq 0 ] && grep -q "RAILWAY SIMULATION" /tmp/test_output.txt; then
    echo -e "${GREEN}  ✓ PASSED${RESET}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}  ✗ FAILED${RESET}"
    FAILED=$((FAILED + 1))
fi
echo ""

echo -e "${YELLOW}Test: No arguments${RESET}"
$EXECUTABLE > /tmp/test_output.txt 2>&1
if [ $? -ne 0 ] && grep -q "Usage:" /tmp/test_output.txt; then
    echo -e "${GREEN}  ✓ PASSED${RESET}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}  ✗ FAILED${RESET}"
    FAILED=$((FAILED + 1))
fi
echo ""

echo -e "${YELLOW}Test: Non-existent file${RESET}"
$EXECUTABLE nonexistent.txt nonexistent2.txt > /tmp/test_output.txt 2>&1
if [ $? -ne 0 ] && grep -q "does not exist" /tmp/test_output.txt; then
    echo -e "${GREEN}  ✓ PASSED${RESET}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}  ✗ FAILED${RESET}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Summary
echo -e "${BLUE}========================================${RESET}"
echo -e "${BLUE}  Test Summary${RESET}"
echo -e "${BLUE}========================================${RESET}"
echo -e "${GREEN}Passed: $PASSED${RESET}"
echo -e "${RED}Failed: $FAILED${RESET}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ✓${RESET}"
    exit 0
else
    echo -e "${RED}Some tests failed!${RESET}"
    exit 1
fi