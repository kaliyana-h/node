#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <stack>
#include "json.hpp"
#include <filesystem>
#include <dirent.h>
#include <algorithm>
 
#define CHAR_SIZE 128
 
using namespace std;
using json = nlohmann::json;
using namespace std::filesystem;
//namespace fs = std::filesystem;
 
map<string, vector<string> > parentTable;
map<string, string > childToParent;
 
// create hash maps: parent to child (one-to-many) and child to parent (one-to-one)
void Insert(string sourceHash, string child, string parent)
{
   // check whether source hash exists in table
   if(!parentTable.count(sourceHash))
       parentTable[sourceHash];
  
   // check whether parent is part of parent table adding the source hash if it is
  
   parentTable[sourceHash].push_back(child);
   if (!parentTable.count(child))
       parentTable[child];
  
   // make child to parent pair
   if(parent == "NULL")
       childToParent.insert(pair<string, string>(sourceHash, "NULL"));
 
   childToParent.insert(pair<string, string>(child, sourceHash));
}
 
// print hash maps
void PrintTables()
{
   for (const auto & c : childToParent)
   {
       cout << c.first << " child of - " << c.second << "\n";
   }
 
   cout << "\n\n";
 
   for (const auto & p : parentTable)
   {
       cout << p.first << " parent of - ";
 
       for (const auto &i : p.second)
       {
           cout << i << ", ";
       }
 
       cout << "\n";
   }
 
   cout << "\n\n";
}
 
// make files out of the created hash maps
void MakeFiles()
{
   ofstream childFile;
   childFile.open("ChildFile.txt");
 
   int mapSizeC = childToParent.size();
   int i = 0;
 
   for (const auto & c : childToParent)
   {
       childFile << c.first << " child of: \n" << c.second;
       if(i < mapSizeC - 1)
       {
           childFile << "\n";
           i++;
       }
   }
   childFile.close();
  
   ofstream parentFile;
   parentFile.open("ParentFile.txt");
 
   bool firstP = true;
  
   for (const auto & p : parentTable)
   {
       bool firstC = true;
      
       if(firstP == false)
       {
           parentFile << "\n";
       }
       parentFile << p.first << " parent of: ";
       firstP = false;
 
       for (const auto &i : p.second)
       {
           if(firstC == false)
           {
               parentFile << "\n";
           }
           parentFile << i;
           firstC = false;
       }
   }
   parentFile.close();
}
 
// rebuild hash map from created files
void RebuildMaps()
{
   ifstream parentFile;
   parentFile.open("ParentFile.txt");
 
   string lineP;
   string parentP;
   string childP;
 
   getline(parentFile, lineP);
 
   while(parentFile.good())
   {
       parentP = lineP.substr(0, 64);
       parentTable[parentP];
       getline(parentFile, lineP);
 
       while(lineP.size() == 64)
       {
           parentTable[parentP].push_back(lineP);
           getline(parentFile, lineP);
       }
   }
   parentFile.close();
 
   ifstream childFile;
   childFile.open("ChildFile.txt");
 
   string lineC;
   string parentC;
   string childC;
 
   getline(childFile, lineC);
 
   while(childFile.good())
   {
       childC = lineC.substr(0, 64);
       getline(childFile, lineC);
       parentC = lineC.substr(0, 64);
       childToParent.insert(pair<string, string>(childC, parentC));
       getline(childFile, lineC);
   }
   childFile.close();
 
   for (const auto & c : childToParent)
   {
       cout << c.first << " child of - " << c.second << "\n";
   }
 
   cout << "\n\n";
 
   for (const auto & p : parentTable)
   {
       cout << p.first << " parent of - ";
 
       for (const auto &i : p.second)
       {
           cout << i << ", ";
       }
 
       cout << "\n";
   }
 
   cout << "\n\n";
}
 
// returns json file of a given hash that has been found using a dfs
void DepthFirstSearch(string sourceHash)
{
   stack<string> stk;
   string topOfStack;
   string parent;
   json jj;
 
   //if(!filesystem::exists("searchfile/" + sourceHash + ".json"))
   //{
       ofstream searchFile("searchfile/" + sourceHash + ".json");
 
       for (auto child : parentTable[sourceHash])
       {
           stk.push(child);
       }   
      
       while (!stk.empty())
       {
           topOfStack = stk.top();
           stk.pop();
 
           parent = childToParent[topOfStack];
 
           ifstream file;
           file.open("jsonfile/" + parent + ".json");
 
           json search = json::parse(file);
           file.close();
 
           jj[topOfStack] = search[topOfStack];
          
           for (auto child : parentTable[topOfStack])
           {
               stk.push(child);
           }           
       }
       searchFile << jj.dump(4);
   //}
}
 
// returns data of a given hash or rotation
string GetData(string inputHash)
{
   string parent;
   string data = "hi";
   string fileName;
 
   // check if given a data hash
   if(filesystem::exists("data/" + inputHash + ".txt"))
   {
       fstream dataFile;
       dataFile.open("data/" + inputHash + ".txt");
       getline(dataFile, data);
       dataFile.close();
       //return;
   }
   // check if given a rotation
   else
   {
       fileName = childToParent[inputHash];
       fstream file;
       file.open("jsonfile/" + fileName + ".json");
       json wholeFile = json::parse(file);
       file.close();
 
       for(const auto& obj : wholeFile)
       {
           if(wholeFile["rotation"] == inputHash)
           {
               data = wholeFile["data"];
               //return;
           }
       }       
   }
   return data;
}
 
// Data structure to store a Trie node
class Trie
{
   public:
 
   bool isLeaf;
 
   // Constructor
   Trie()
   {
       this->isLeaf = false;
   }
 
   map<char, Trie*> children;
 
   // Iterative function to insert a key into a Trie
   void InsertChars(string key)
   {
       //char originalNode = NULL;
 
       // start from the root node
       Trie* curr = this;
       //children[originalNode].push_back[curr];
 
       for (int i = 0; i < key.length(); i++)
       {
           // check whether root node exists and add node into map
           if(!curr->children[key[i]]){
               curr->children[key[i]] = new Trie();
           }
  
           // go to the next node
           curr = curr->children[key[i]];
       }
       // mark the current node as a leaf
       curr->isLeaf = true;
       //if (curr->isLeaf) cout << "word here" << endl;
   }
 
   // returns a list of keys matching the prefix
   vector<string> MatchingPrefix(string prefix)
   {
       vector<string> matchingKeys;
 
       int i;
       int len = prefix.length();
      
       Trie* curr = this;
 
       for (i = 0; i < len; i++)
       {
           // go to the next node
           curr = curr->children[prefix[i]];
  
           // if the string is invalid (reached end of a path in the Trie)
           if (curr == nullptr)
           {
               break;
           }
       }
      
       stack<string> nodeStack;
       stack<Trie*> prefixes;
      
       string Prefix;
 
       nodeStack.push(prefix);
       prefixes.push(curr);
 
       while (!nodeStack.empty())
       {
           Prefix = nodeStack.top();
           nodeStack.pop();
 
           Trie *curr = prefixes.top();
           prefixes.pop();
 
           // cout << Prefix << "\n";
           if (curr->isLeaf)
           {
               matchingKeys.push_back(Prefix);
           }   
           for (auto child : curr->children)
           {
               nodeStack.push(Prefix + child.first);
               prefixes.push(child.second);               
           }                     
       }
       // for (auto v : matchingKeys)
       //     cout << v << "\n";
 
       return matchingKeys;
   }
 
   // returns data from a given partial hash
   vector<string> GetPrefixData(string partialHash)
   {
       string dataHash;
       string data;
       string fileName;
       vector<string> matchingData;
 
       int len = partialHash.length();
 
       dataHash = partialHash;
      
       Trie* curr = this;
       for (int i = 0; i < len; i++)
       {
           // go to the next node
           curr = curr->children[partialHash[i]];
  
           // if the string is invalid (reached end of a path in the Trie)
           if (curr == nullptr) {
               break;
           }
       }
  
       stack<string> nodeStack;
       stack<Trie*> prefixes;
      
       nodeStack.push(partialHash);
       prefixes.push(curr);
 
       while (!nodeStack.empty())
       {
           dataHash = nodeStack.top();
           nodeStack.pop();
 
           Trie *curr = prefixes.top();
           prefixes.pop();
          
           if (curr->isLeaf)
               matchingData.push_back(dataHash);
          
           for (auto child : curr->children)
           {
               nodeStack.push(dataHash + child.first);
               prefixes.push(child.second);           
           }                     
       }
 
       return matchingData;
   }
   void makeJsonTries()
   {
       DIR *dir;
       struct dirent *diread;
       vector<string> files;
       string fileName;
       string str;
 
       if ((dir = opendir("jsonfile/")) != NULL) {
           while ((diread = readdir(dir)) != NULL) {
               fileName = diread->d_name;
               if(fileName.substr(fileName.find_last_of(".")) == ".json")
                   files.push_back(fileName);
           }
           closedir (dir);
       }
 
       string fType = ".json";
 
       for (auto file : files)
       {
           try {
               string jsonName;
               stringstream jsonFile;
               jsonFile << file;
               jsonFile >> jsonName;
 
               std::string::size_type i = jsonName.find(fType);
 
               if (i != std::string::npos)
                   jsonName.erase(i, fType.length());
 
               fstream ff;
               ff.open("jsonfile/" + jsonName + ".json");
               json jp = json::parse(ff);
               ff.close();
               for(const auto& obj : jp)
               {
                   str = obj["rotation"];
                   // cout << str << "\n";
                   InsertChars(str);
               }
           } catch (std::exception ex) {
               std::cout << "I hung on a json parse error: ";
               std::cout << ex.what();
               std::cout << "\n";
           }
       }
   }
   void makeDataTries()
   {
       DIR *dir;
       struct dirent *diread;
       vector<string> files;
       string fileName;
       string data;
 
       if ((dir = opendir("data/")) != NULL) {
           while ((diread = readdir(dir)) != NULL) {
               fileName = diread->d_name;
               if(fileName.substr(fileName.find_last_of(".")) == ".txt")
                   files.push_back(fileName);
           }
           closedir (dir);
       }
 
       string fType = ".txt";
 
       for (auto file : files)
       {
           try {
               string dataFileName;
               stringstream dataFile;
               dataFile << file;
               dataFile >> dataFileName;
 
               std::string::size_type i = dataFileName.find(fType);
 
               if (i != std::string::npos)
                   dataFileName.erase(i, fType.length());
              
               InsertChars(dataFileName);
           } catch (std::exception ex) {
               std::cout << "I hung on a file parse error: ";
               std::cout << ex.what();
               std::cout << "\n";
           }
       }
   }
};