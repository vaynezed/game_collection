workdir=$(cd $(dirname $0); pwd)
echo ${workdir}
project_dir="${workdir}/../"
include_dir="${project_dir}/include/"
src_dir="${project_dir}/src/"
find ${src_dir}  -name "*.cpp" -type f | xargs  clang-format  -style=webkit -i
find ${include_dir}  -name "*.hpp" -type f | xargs clang-format  -style=webkit -i 
