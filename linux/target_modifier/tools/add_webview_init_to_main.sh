#!/bin/bash

# Adds
# initCEFProcesses(argc, argv);
# to main function of main.cc

# Check if the correct number of arguments is passed
if [ $# -ne 1 ]; then
  echo "Usage: $0 <file-path>"
  exit 1
fi

# Assign file path argument
file="$1"

# Check if the file exists at the provided path
if [ ! -f "$file" ]; then
  # If not, construct a path in the parent directory with the same filename
  parent_dir_file=$(dirname "$(dirname $file)")/$(basename "$file")

  # Check if the file exists at the new path
  if [ -f "$parent_dir_file" ]; then
    file="$parent_dir_file" # Use the new path if it exists
  else
    # If neither path is valid, exit
    echo "File not found at '$file' or in its parent directory."
    exit 1
  fi
fi

# Check if the line initCEFProcesses(argc, argv); already exists in the file
if grep -q '^[[:space:]]*int exit_code = initCEFProcesses(argc, argv);' "$file"; then
  echo "The line 'int exit_code = initCEFProcesses(argc, argv);' already exists in the file."
else
  # remove "  initCEFProcesses(argc, argv);"
  sed -i '/^[[:space:]]*initCEFProcesses(argc, argv);/d' "$file"

  # Insert int exit_code = initCEFProcesses(argc, argv); before the first line starting with g_autoptr(
  awk '
    /^[[:space:]]*g_autoptr/ {
      print "  int exit_code = initCEFProcesses(argc, argv);";
      print "  if (exit_code >= 0) {";
      print "    return exit_code;";
      print "  }";
    }
    { print }
  ' "$file" > "$file.tmp" && mv "$file.tmp" "$file"

  echo "Added 'int exit_code = initCEFProcesses(argc, argv);' before the first line starting with 'g_autoptr('."
fi
