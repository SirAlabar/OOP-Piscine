#!/bin/bash

DEST="all_sources_flat"

mkdir -p "$DEST"

echo "Collecting all .hpp and .cpp files into a single folder..."

find . -type f \( -name "*.hpp" -o -name "*.cpp" \) -exec cp {} "$DEST" \;

echo "Done! All files are now in ./$DEST"
