#!/bin/bash
################################################################################
# rebuild.sh - Rebuild ICARUS OS Core from scratch
# 
# This script performs a clean rebuild by calling 'make rebuild'
# which cleans all artifacts and rebuilds everything including:
# - Binary files (elf, hex, bin)
# - Static analysis reports
# - Test coverage reports
#
# Usage:
#   ./rebuild.sh
################################################################################

set -e  # Exit on error

# Get script directory (where this script is located)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Call make rebuild
make rebuild
