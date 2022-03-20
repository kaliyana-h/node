#include "httplib.h"
#include "json.hpp"
#include "sha256.cpp"
#include "Taskflow.cpp"
#include <iostream>
#include <string>
#include <map>
 
 
using namespace std;
using namespace httplib;
using json = nlohmann::json;
 
int main()
{
   httplib::Client cli("localhost", 2180);
 
   int selection;
 
   cout << "Select 1 for Post, 3 for Prefix Search, 5 for Get Data, 7 for Partial Hash, 0 for Get Index, 8 for Hash Wall, or 4 for Automine: ";
   cin >> selection;
  
   if(selection == 1)
   {
       string source;
       string data;
       string target;
       string iteration;
 
       cout << "Source: ";
       cin >> source;
       cout << "Data: ";
       cin >> data;
       cout << "Target: ";
       cin >> target;
       cout << "Iterations: ";
       cin >> iteration;
 
       json j;
 
       j["source"] = source;
       j["data"] = data;
       j["target"] = target;
       j["iteration"] = stoi(iteration);
 
       cli.Post("/api/v2/mine", j.dump(), "text/plain");
   }
  
   if(selection == 0)
   {
       string source;
 
       cout << "Index: ";
       cin >> source;
 
       string path = "/api/v2/index/" + source;
 
       auto res = cli.Get(path.c_str());  
 
       cout << res->body << endl;
 
   } 
   if(selection == 5)
   {
       string data;
 
       cout << "Data: ";
       cin >> data;
     
       string path = "/api/v2/data/" + data;
 
       auto res = cli.Get(path.c_str()); 
 
       cout << "\"" + res->body + "\"" << endl;
   }
   if(selection == 3)
   {
       string prefix;
 
       cout << "Prefix: ";
       cin >> prefix;
 
       string path = "/api/v2/filter/" + prefix;
 
       auto res = cli.Get(path.c_str());  
 
       cout << "\"" + res->body + "\"" << endl;
 
   } 
   if(selection == 7)
   {
       string hash;
 
       cout << "Partial Hash: ";
       cin >> hash;
 
       string path = "/api/v2/hash/" + hash;
 
       auto res = cli.Get(path.c_str());  
 
       cout << "\"" + res->body + "\"" << endl;
 
   } 
   if(selection == 8)
   {
       map<string, vector<string> > parentTable;
       map<string, string > childToParent;
      
       string source;
 
       cout << "Source: ";
       cin >> source;
 
       SHA256 sha256;
       string sourceHash;
 
       if(size(source) == 64)
       {
           sourceHash = source;
       }
       else
       {
           sourceHash = sha256(source);
       }
 
       string retIndex;
       json index;  
 
       string path = "/api/v2/index/" + sourceHash;
 
       auto res = cli.Get(path.c_str());  
 
       retIndex = res->body;
 
       index = json::parse(retIndex);
 
       for (const auto& obj : index)
       {
           string child = obj["rotation"];
           string source = obj["source"];
          
           if(!parentTable.count(source))
               parentTable[source];
              
           parentTable[source].push_back(child);
 
           if (!parentTable.count(child))
               parentTable[child];
          
           // make child to parent pair
           if(source == sourceHash)
               childToParent.insert(pair<string, string>(source, "NULL"));
 
           childToParent.insert(pair<string, string>(child, source));
       }
 
       map<string, int > workDone;
       stack<string> stk;
       string topOfStack;
       string parent;
       string parIndex;
       int work = 0;
       int currLength;
       json jj;
 
       for (auto child : parentTable[sourceHash])
       {
           stk.push(child);
       }
      
       while (!stk.empty())
       {
           topOfStack = stk.top();
           stk.pop();
 
           parent = childToParent[topOfStack];
 
           jj[topOfStack] = index[topOfStack];
 
          
           for (auto chi : parentTable[topOfStack])
           {
               for(const auto& obj : jj)
               {
                   string curr = obj["target"];
                   currLength = curr.size();
               }
              
               work += pow(16, currLength);
              
               workDone.insert(pair<string, int>(chi, work));
 
               stk.push(chi);
           }
       }
      
 
       string mostWorkedR;
       int mostWorkedC;
       string dataHash;
       string SH;
       string preCost;
       string rot;
       string beforeHash;
       string costFinal;
       string user;
       uint64_t nonce = 0;
 
       for (const auto & w : workDone)
       {
           if(w.second > mostWorkedC)
           {
               mostWorkedR = w.first;
               mostWorkedC = w.second;
           }
       }
 
       string s;
       json ind; 
 
       cout << "most worked rotation: " << mostWorkedR << "\n";
 
       for (const auto& obj : index)
       {
           if (obj["rotation"] == mostWorkedR)
           {
               dataHash = obj["datahash"];
               preCost = obj["target"];
               user = obj["user"];
           }
       }
 
       string beforeCost;
       string finalCost;
 
       nonce = Taskflow(mostWorkedR, dataHash, user, preCost);
       beforeCost = mostWorkedR + dataHash + preCost + user + to_string(nonce);
       finalCost = sha256(beforeCost);
 
       cout << "final cost: " << finalCost << "\n";
 
       json j;
 
       j[finalCost] = {
               {"datahash", dataHash},
               {"n", to_string(nonce)},
               {"rotation", finalCost},
               {"source", mostWorkedR},
               {"target", preCost},
               {"cost", finalCost},
               {"user", user}
           };
 
       cout << j.dump(4) << "\n";
 
       cli.Post("/api/v2/data", j.dump(4), "text/plain");
   }
   if(selection == 4)
   {
       string source;
       string data;
       string target;
       string iteration;
 
       cout << "Source: ";
       cin >> source;
       cout << "Data: ";
       cin >> data;
       cout << "Target: ";
       cin >> target;
       cout << "Iterations: ";
       cin >> iteration;
 
       json j;
 
       j["source"] = source;
       j["data"] = data;
       j["target"] = target;
       j["iteration"] = stoi(iteration);
      
       cli.Post("/api/v2/job", j.dump(), "text/plain");
   }
   else
   {
       cout << "Invalid input";
   } 
 
   return 0;
}
// fc5669b52ce4e283ad1d5d182de88ff9faec6672bace84ac2ce4c083f54fe2bc