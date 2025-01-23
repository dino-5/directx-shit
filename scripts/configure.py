import os

def find_cpp_and_c_files(directory):
    cpp_c_files = []

    # os.walk automatically handles directory traversal
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.c')):
                cpp_c_files.append(os.path.join(root, file))
    
    return cpp_c_files

compiler = "clang++"
flags = "-I./source -Wall -std=c++20"
project_root = os.getcwd()
output_file = f"{project_root}/compile_commands.json"

with open(output_file, 'w') as file:
    file.write("[\n")
    sourceFiles = find_cpp_and_c_files(f"{project_root}/source")
    for src in sourceFiles:
        src = src.replace("\\", "/")
        file.write('\t{\n')
        file.write(f'\t\t"directory": "{project_root}",\n')
        file.write(f'\t\t"command": "{compiler} {flags} -c {src}",\n')
        file.write(f'\t\t"file": "{src}",\n')
        file.write('\t}\n')
    file.write("]")
