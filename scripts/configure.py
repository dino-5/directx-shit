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

sourceDir = "./source"
thirdPartyDir = f"{sourceDir}/third_party"
imguiDir = f"{thirdPartyDir}/imgui"
fmtDir = f"{thirdPartyDir}/fmt/include"

packageDir = "./generated/packages/"

dx12_dir=f"{packageDir}/Microsoft.Direct3D.D3D12.1.611.2/build/native"
dx12_include_dir = f"{dx12_dir}/include"
dx12_lib_dir = f"{dx12_dir}/bin/x64"

dxc_dir = f"{packageDir}/Microsoft.Direct3D.DXC.1.7.2308.12/build/native"
dxc_include_dir = f"{dxc_dir}/include"
dxc_lib_dir = f"{dxc_dir}/lib/x64"

flags = f"-I{sourceDir} -I{imguiDir} -I{fmtDir} -I{dxc_include_dir} -I{dx12_include_dir} -Wall -std=c++20"
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
