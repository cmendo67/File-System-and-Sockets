#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/wait.h>
using namespace std;

class myFile {

    public:
    string name;
    string data;
    bool operator == (const myFile& s) const { 
        return name == s.name && data == s.data; 
    }
    bool operator != (const myFile& s) const { 
        return !operator==(s); 
    }

    myFile(string n, string d) {
        name = n;
        data = d;
    }

    myFile(string n) {
        name = n;
    }

    string getData() {
        return data;
    }

    void setData(string text) {
        data = text;
    }

    int getFileSize() {
        int s = data.length();
        return s;
    }

    string getName() {
        return name;
    }

};