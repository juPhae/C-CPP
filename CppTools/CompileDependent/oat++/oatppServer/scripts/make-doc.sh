#!/bin/bash
# 检测nodejs
NODEJS_V=$(node -v)
NODEJS_V_
echo "nodejs version : "${NODEJS_V}
if [[ $NODEJS_V < "v16" ]]; then
    # passwd需要替换成正确的密码
    echo passwd | sudo -S n 16
    NODEJS_V_=$(node -v)
fi
echo "nodejs version : "${NODEJS_V_}
#生成文档
cd .. && apidoc -i src/ -o docs/apidoc/
if [[ $NODEJS_V > "v9" ]]; then
    # passwd需要替换成正确的密码
    echo passwd | sudo -S n 8
fi
