 #!/bin/bash

# Define the target directory
directory="./assets/shaders"


# Check if the target is not a directory
if [ ! -d "$directory" ]; then
  exit 1
fi

# Loop through files in the target directory
for file in "$directory"/*; do
  if [ -f "$file" ]; then
    shader_path="$directory/"
    if [[ $file == *".frag"* ]]; then
        shader_name_ext="${file/$shader_path/}"
        shader_name="${shader_name_ext/.frag/}"
        cmd="glslc -c $file -o ./bin/Debug/$shader_name.spv"
        echo "Compiling shader $shader_name_ext..."
        $cmd
    fi
    if [[ $file == *".vert"* ]]; then
        shader_name_ext="${file/$shader_path/}"
        shader_name="${shader_name_ext/.vert/}"
        cmd="glslc -c $file -o ./bin/Debug/$shader_name.spv"
        echo "Compiling shader $shader_name_ext..."
        $cmd
    fi
  fi
done