#!/bin/bash
# 检测nodejs
NODEJS_V=$(N_V=$(node -v) && echo ${N_V:1})
echo "node version : "${NODEJS_V}
if [[ $NODEJS_V < "16" ]]; then
    echo "The version is too low.Reset nodejs's version"
    # passwd需要替换成正确的密码
    echo passwd | sudo -S n 16
    CHANGE_NODE_V=true
    NODEJS_V_=$(N_V=$(node -v) && echo ${N_V:1})

fi
#生成文档
cd .. && apidoc -i src/ -o docs/apidoc/

if [[ $CHANGE_NODE_V == "true" ]]; then
    echo "Reset nodejs's version"

    echo "nodejs version : "${NODEJS_V_}
    # passwd需要替换成正确的密码
    echo passwd | sudo -S n $NODEJS_V
fi
