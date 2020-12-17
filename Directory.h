#include "myFile.h"
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

class Directory {
    public:
    std::vector<myFile> fileList;
    string name;

    Directory() {
        
    }

    Directory(string n) {
        name = n;
        fileList;
    } 

    void createFile(myFile f) {
        fileList.push_back(f);
    }

    int size() {
        int s = fileList.size();
        return s;
    }

    void deleteFile(myFile f) {
        for (vector<myFile>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
            if (it->getName() == f.getName()) {
                fileList.erase(it);
                break;
            }
        }
    }
                    //test.txt, hello
    void changeData(myFile f, string newData){
        for(int i = 0; i < fileList.size(); i++){
            if(fileList.at(i).getName() == f.getName()){
                cout <<"Data: " << fileList.at(i).getData() << endl;
                fileList.at(i).setData(newData);
            }
        }
    }

    int findElement(myFile f){
        for(int i = 0; i < fileList.size(); i++){
            if(fileList.at(i).getName() == f.getName()){
                return i;
            }
        }
    }

    string fileListing(){
        string result = "\nDirectory Listing:\n";
        for(int i = 0; i < fileList.size(); i++){
            result += "Name: ";
            result += fileList.at(i).getName();
            result += "\n";
        }
        return result;
    } 

    string directoryListings(){
        string result = "\nDirectory Listing:\n";
        for(int i = 0; i < fileList.size(); i++){
            result += "Name: ";
            result += fileList.at(i).getName();
            result += "\nFile length: ";
            result += to_string(fileList.at(i).getFileSize());
            result += "\n";
        }  
        return result;
    }

    string readData(myFile f){
        for(int i = 0; i < fileList.size(); i++){
            if(fileList.at(i).getName() == f.getName()){
                return fileList.at(i).getData();
            }
        }
    }

    int fileSize(myFile f) {
        for(int i = 0; i < fileList.size(); i++){
            if(fileList.at(i).getName() == f.getName()){
                return fileList.at(i).getFileSize();
            }
        }
    }

    bool exists(myFile f) {
        for(int i = 0; i < fileList.size(); i++){
            if(fileList.at(i).getName() == f.getName()){
                return true;
            }
        }
        return false;
    }

    string getDirectoryName() {
        return name;
    }

};