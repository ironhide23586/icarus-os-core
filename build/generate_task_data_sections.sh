#!/bin/bash
# build/generate_task_sections.sh
# Generates task data sections in linker script based on ICARUS_MAX_TASKS

set -e  # Exit on error

# Paths
CONFIG_H="Core/Inc/icarus/config.h"
LINKER_SCRIPT="STM32H750VBTX_FLASH.ld"
LINKER_BACKUP="${LINKER_SCRIPT}.backup"

# Extract ICARUS_MAX_TASKS value from config.h
MAX_TASKS=$(grep "#define ICARUS_MAX_TASKS" "$CONFIG_H" | head -1 | awk '{print $3}')

if [ -z "$MAX_TASKS" ]; then
    echo "Error: Could not find ICARUS_MAX_TASKS in $CONFIG_H"
    exit 1
fi

echo "Found ICARUS_MAX_TASKS = $MAX_TASKS"

# Backup original linker script if not already backed up
if [ ! -f "$LINKER_BACKUP" ]; then
    echo "Creating backup: $LINKER_BACKUP"
    cp "$LINKER_SCRIPT" "$LINKER_BACKUP"
fi

# Check if task sections already exist
if grep -q "\.task_data_0" "$LINKER_SCRIPT"; then
    echo "Task sections already exist in linker script"
    echo "Removing old task sections..."
    
    # Remove old task sections (between markers)
    sed -i.tmp '/\/\* BEGIN GENERATED TASK SECTIONS \*\//,/\/\* END GENERATED TASK SECTIONS \*\//d' "$LINKER_SCRIPT"
    rm -f "${LINKER_SCRIPT}.tmp"
fi

# Find insertion point (before the closing brace of SECTIONS)
# We'll insert before the .ARM.attributes section or before final closing brace

# Generate task data sections to a temporary file
TEMP_SECTIONS=$(mktemp)

cat > "$TEMP_SECTIONS" << 'EOF'

  /* BEGIN GENERATED TASK SECTIONS */
  /* Task isolated data sections - Generated from ICARUS_MAX_TASKS */
EOF

for ((i=0; i<MAX_TASKS; i++)); do
    cat >> "$TEMP_SECTIONS" << EOF

  .task_data_$i (NOLOAD) :
  {
    . = ALIGN(4096);
    _task_data_${i}_start = .;
    *(.task_data_$i)
    . = ALIGN(4096);
    _task_data_${i}_end = .;
  } >RAM_D1
EOF
done

cat >> "$TEMP_SECTIONS" << 'EOF'

  /* END GENERATED TASK SECTIONS */
EOF

# Insert before .ARM.attributes or before final closing brace
if grep -q "\.ARM\.attributes" "$LINKER_SCRIPT"; then
    # Insert before .ARM.attributes
    awk '/\.ARM\.attributes/ && !inserted { system("cat '"$TEMP_SECTIONS"'"); inserted=1 } { print }' "$LINKER_SCRIPT" > "${LINKER_SCRIPT}.new"
else
    # Insert before final closing brace of SECTIONS
    awk '/^}$/ && !inserted { system("cat '"$TEMP_SECTIONS"'"); inserted=1 } { print }' "$LINKER_SCRIPT" > "${LINKER_SCRIPT}.new"
fi

# Clean up temp file
rm -f "$TEMP_SECTIONS"

# Replace original with new version
mv "${LINKER_SCRIPT}.new" "$LINKER_SCRIPT"

echo "Successfully generated $MAX_TASKS task data sections in $LINKER_SCRIPT"
echo "Backup saved as: $LINKER_BACKUP"
