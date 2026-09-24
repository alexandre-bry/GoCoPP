#!/bin/bash
# Setup script for GoCoPP - applies CGAL patch for Boost 1.86+ compatibility
# This script is automatically run by pixi before building

set -e

echo "Setting up GoCoPP environment..."

# Determine the pixi environment directory
if [ -n "$PIXI_PROJECT_ROOT" ]; then
    # Running via pixi task - env is in .pixi/envs/default
    PIXI_ENV_DIR="$PIXI_PROJECT_ROOT/.pixi/envs/default"
elif [ -n "$1" ]; then
    # Called with explicit path
    PIXI_ENV_DIR="$1"
else
    # Try to find it from script location
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PIXI_ENV_DIR="$SCRIPT_DIR/.pixi/envs/default"
fi

if [ -z "$PIXI_ENV_DIR" ] || [ ! -d "$PIXI_ENV_DIR" ]; then
    echo "Warning: Could not determine PIXI_ENV_DIR, skipping patch"
    exit 0
fi

# Path to the CGAL iterator header
ITERATOR_H="$PIXI_ENV_DIR/include/CGAL/boost/graph/iterator.h"

if [ ! -f "$ITERATOR_H" ]; then
    echo "Warning: CGAL iterator.h not found at $ITERATOR_H, skipping patch"
    exit 0
fi

# Check if already patched
if grep -q "return (g != nullptr)" "$ITERATOR_H"; then
    echo "CGAL already patched"
    exit 0
fi

# Apply the patch using sed - replaces the problematic base() calls
echo "Patching CGAL for Boost 1.86+ compatibility..."
sed -i 's/return (! (this->base() == nullptr));/return (g != nullptr);/g' "$ITERATOR_H"

echo "CGAL patched successfully!"
