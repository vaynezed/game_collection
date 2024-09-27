workdir=$(cd $(dirname $0); pwd)
echo ${workdir}
project_dir="${workdir}/../"
find ${project_dir} -name "*.cpp"  -o -name "*.hpp" | xargs cat |
     wc -l 
