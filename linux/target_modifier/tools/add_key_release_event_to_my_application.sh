#!/bin/bash

# Add
# g_signal_connect(view, \"key_release_event\", G_CALLBACK(processKeyEventForCEF), nullptr);
# after
# FlView* view = fl_view_new(project);
# if not already added

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

# Check if the line g_signal_connect exists in the file
if grep -q '^[[:space:]]*g_signal_connect(view, "key_release_event", G_CALLBACK(processKeyEventForCEF), nullptr);' "$file"; then
  echo "The line 'g_signal_connect(view, \"key_release_event\", G_CALLBACK(processKeyEventForCEF), nullptr);' already exists in the file."
else
  # Insert g_signal_connect after the first occurrence of FlView* view = fl_view_new(project);
  awk '
    /^[[:space:]]*FlView\* view = fl_view_new\(project\);/ {
      print $0;
      print "  g_signal_connect(view, \"key_release_event\", G_CALLBACK(processKeyEventForCEF), nullptr);";
      next;
    }
    { print }
  ' "$file" > "$file.tmp" && mv "$file.tmp" "$file"

  echo "Added 'g_signal_connect(view, \"key_release_event\", G_CALLBACK(processKeyEventForCEF), nullptr);' after 'FlView* view = fl_view_new(project);'."
fi
