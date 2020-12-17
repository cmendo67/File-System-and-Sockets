#include "Directory.h"
#include <string.h>
#include <vector>
#include <algorithm>
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
#include <vector>
#include <list>
using namespace std;

class MainDirectory {

    public:
    std::vector<Directory> direct_List;

    MainDirectory() {
        direct_List;
    } 
    // add a directory
    void createDirectory(Directory d) {
        direct_List.push_back(d);
    }

    int size() {
        int s = direct_List.size();
        return s;
    }


    int getNewPos(Directory d) {
        for(int i = 0; i < direct_List.size(); i++){
            if(direct_List.at(i).getDirectoryName() == d.getDirectoryName()){
                return i;
            }
        }
    }

    void deleteDirectory(Directory d) {
        for (vector<Directory>::iterator it = direct_List.begin(); it != direct_List.end(); ++it) {
            if (it->getDirectoryName() == d.getDirectoryName()) {
                direct_List.erase(it);
                int n = 1;
                break;
            }
        }
    } 

    bool directoryExists(Directory d) {
         for(int i = 0; i < direct_List.size(); i++){
            if(direct_List.at(i).getDirectoryName() == d.getDirectoryName()){
                return true;
            }
        }
        return false;
    }
};
