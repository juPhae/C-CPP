PROJECT_ROOT_DIR="$(cd "$(dirname $0)" && cd .. && pwd)"
echo $PROJECT_ROOT_DIR

# 编译整个项目
./build.sh

# 构建部署文件
cd $PROJECT_ROOT_DIR

mkdir -p deploy && cd deploy
mkdir -p oatpp-server && cd oatpp-server
mkdir -p lib
mkdir -p config
cd -

cp $PROJECT_ROOT_DIR/install/oatppServer ./oatpp-server
cp -r $PROJECT_ROOT_DIR/config/ ./oatpp-server

cd oatpp-server && chmod 777 oatppServer
