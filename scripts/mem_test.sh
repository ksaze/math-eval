#!/bin/bash

EXEC=./eval
VALGRIND="valgrind --quiet --error-exitcode=1"

# List of test inputs
inputs=(
  "42 + 69"
  "1 + (2 * 3)"
  "invalid input"
  "999 / 3 - 1"
)

status=1 # binary true

# Check if eval exists
if [ ! -f "$EXEC" ]; then
  echo "Error: $EXEC not found. Please build the eval executable."
  exit 1
fi

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
  echo "Error: Valgrind not installed. Please install it."
  exit 1
fi

for input in "${inputs[@]}"; do
  echo -n "Testing: '$input' ... "
  
  # Run Valgrind and capture output, ignoring eval's exit code
  $VALGRIND "$EXEC" "$input" --suppress-output > /dev/null  2>&1
  ret=$?
  if [ $ret -eq 1 ]; then
    echo "FAIL"
  else
    echo "PASS"
    status=0 # binary false
  fi
done

exit $status
