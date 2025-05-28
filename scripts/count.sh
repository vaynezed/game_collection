workdir=$(cd $(dirname $0); pwd)
echo ${workdir}
project_dir="${workdir}/../"
find ${project_dir}/src/ -name "*.cpp"  -o -name "*.hpp" | xargs cat |
     wc -l 
find ${project_dir}/include/ -name "*.cpp"  -o -name "*.hpp" | xargs cat |
     wc -l 
