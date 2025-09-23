#!/bin/bash

# Adds '#include <webview_cef/webview_cef_plugin.h>' at the top of the file

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

# Check if the line already exists in the file
if grep -q '#include <webview_cef/webview_cef_plugin.h>' "$file"; then
  echo "The line '#include <webview_cef/webview_cef_plugin.h>' already exists in the file."
else
  # Add the line at the top of the file
  (echo "#include <webview_cef/webview_cef_plugin.h>"; cat "$file") > "$file.tmp" && mv "$file.tmp" "$file"
  echo "Added '#include <webview_cef/webview_cef_plugin.h>' at the top of the file."
fi
