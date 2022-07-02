```c++
//遍历map的三种方式

#include <iostream>
#include <map>
#include <string>

using namespace std;

int main() {
  map<string, string> m = {{"A0", "red"}, {"A1", "green"}, {"A2", "yellow"}, {"A3", "blue"}};

  map<string, string>::iterator it;

  cout << "方式一：" << endl;
  for (auto &t : m) {
    cout << "key:" << t.first << " value:" << t.second << endl;
  }

  cout << "方式二：" << endl;
  for (map<string, string>::iterator iter = m.begin(); iter != m.end(); ++iter) {
    cout << "key:" << iter->first << " value:" << iter->second << endl;
  }

  cout << "方式三：" << endl;
  map<string, string>::iterator iter = m.begin();
  while (iter != m.end()) {
    cout << "key:" << iter->first << " value:" << iter->second << endl;
    ++iter;
  }
}
```

