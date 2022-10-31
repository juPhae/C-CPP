PROJECT_ROOT_DIR="$(cd "$(dirname $0)" && cd .. && pwd)"
cd $PROJECT_ROOT_DIR
mkdir -p build && cd ./build
cmake .. && make && make install
