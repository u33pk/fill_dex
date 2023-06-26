#include <iostream>
#include <fstream>

using namespace std;
int main(){
    fstream conf_fs("/home/aosp/Project/dex2fill/test/test_getline.cpp", ios::in);
    if(conf_fs.good()){
        string _line;
        while (getline(conf_fs, _line, '\n'))
        {
            /* code */
            cout << _line << endl;
        }
        
    }
    return 0;
}