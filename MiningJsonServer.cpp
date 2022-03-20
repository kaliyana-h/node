#include "sha256.cpp"
#include "httplib.h"
#include "json.hpp"
#include "Table.cpp"
#include "Taskflow.cpp"
//#include "Trie.cpp"
#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <cstddef>
#include <map>
#include <random>
#include <filesystem>
#include <dirent.h>
#include <algorithm>
 
using namespace std;
using namespace httplib;
namespace fs = std::filesystem;
using json = nlohmann::json;
 
struct Block
{
   std::string source;
   std::string data;
   std::string target;
   std::string iteration;
};
 
void ToMine(Block Mine);
 
int main()
{
   Server svr;
 
   svr.Post("/api/v2/mine", [&](const Request &req, Response &res, const ContentReader &content_reader) {
       res.set_header("Access-Control-Allow-Origin", "*");
      
       struct Block Mine;
 
       if (req.has_param("source"))
       {
           cout << "found source" << "\n";
           Mine.source = req.get_param_value("source");
 
           Mine.data = "";
           Mine.target = "";
           Mine.iteration = "1";
          
           if (req.has_param("data")) {
               Mine.data = req.get_param_value("data");
           }
 
           if (req.has_param("target")) {
               Mine.target = req.get_param_value("target");
           }
 
           if (req.has_param("iteration")) {
               Mine.iteration = req.get_param_value("iteration");
           }
 
           if ((Mine.data == "") || (Mine.target == ""))
           {
               string parent = childToParent[Mine.source];
 
               fstream file;
               file.open("jsonfile/" + parent + ".json");
               json File = json::parse(file);
               file.close();
 
               json jj = File[Mine.source];
 
               Mine.data = jj["data"];
               Mine.target = jj["target"];
           }
           ToMine(Mine);
       }
      
       else
       {
           cout << "didn't find 'source'" << "\n";
           std::string body;
           content_reader([&](const char *data, size_t data_length) {
               body.append(data, data_length);
               return true;
           });
 
           json j = json::parse(body);
           // test
           cout << j.dump(4);
 
           Mine.source = j["source"];
           Mine.data = j["data"];
           Mine.target = j["target"];
           Mine.iteration = std::to_string((int)j["iteration"]);
 
           ToMine(Mine);
       }
   });
   svr.Get(R"(/api/v2/index/(.*))", [](const Request &req, Response &res) {
       res.set_header("Access-Control-Allow-Origin", "*");
       const string source = req.matches[1];
 
       json index;
 
       //cout << "source: " << source << "\n";
 
       if(filesystem::exists("jsonfile/" + source + ".json"))
       {
           DepthFirstSearch(source);
           fstream searchFile;
           searchFile.open("searchfile/" + source + ".json");
           index = json::parse(searchFile);
           searchFile.close();
           //cout << index.dump(4) << "\n";
       }
       else
       {
           index = json({});
       }
 
       res.set_content(index.dump(4), "text/plain");
   });
 
   svr.Get(R"(/api/v2/data/(.*))", [&](const Request &req, Response &res) {
       res.set_header("Access-Control-Allow-Origin", "*");
       const string source = req.matches[1];
 
       if(source.length() == 64)
       {
           string data;
 
           data = GetData(source);
 
           res.set_content(data, "text/plain");
       }
 
       else
       {
           vector<string> dataFile;
           Trie insert;
           string data;
           string allData = " \n";
 
           insert.makeDataTries();    
 
           dataFile = insert.GetPrefixData(source);
           for (auto v : dataFile)
           {
               fstream File;
               File.open("data/" + v + ".txt");
               getline(File, data);
               allData.append(data + "\n");
               File.close();
           }
          
           res.set_content(allData, "text/plain");
       } 
   });
 
   svr.Get(R"(/api/v2/filter/(.*))", [&](const Request &req, Response &res) {
       res.set_header("Access-Control-Allow-Origin", "*");
       const string source = req.matches[1];
 
       vector<string> matchingKeys;
       Trie insert;
       string vecStr = "";     
      
       insert.makeJsonTries();
 
       matchingKeys = insert.MatchingPrefix(source);
 
       for (auto v : matchingKeys)
       {
           vecStr.append(v + "\n");
           //cout << vecStr;
       }
       res.set_content(vecStr, "text/plain");
   });
 
   svr.Get(R"(/api/v2/hash/(.*))", [&](const Request &req, Response &res) {
       res.set_header("Access-Control-Allow-Origin", "*");
       const string source = req.matches[1];
 
       vector<string> dataFile;
       Trie insert;
       string data;
       string allData = " \n";
 
       insert.makeDataTries();    
 
       dataFile = insert.GetPrefixData(source);
       for (auto v : dataFile)
       {
           fstream File;
           File.open("data/" + v + ".txt");
           getline(File, data);
           allData.append(data + "\n");
           File.close();
       }
      
       res.set_content(allData, "text/plain");
 
   });
 
   svr.Post("/api/v2/data", [&](const Request &req, Response &res, const ContentReader &content_reader) {
       res.set_header("Access-Control-Allow-Origin", "*");
       std::string source;
       content_reader([&](const char *data, size_t data_length) {
           source.append(data, data_length);
           return true;
       });
 
       SHA256 sha256;
 
       //struct Block Mine;
       string beforeCost;
       string finalCost;
      
       json j = json::parse(source);
       // test
       // cout << j.dump(4) << "\n";
 
       string s;
       string sh;
       string d;
       string t;
       string u;
       string n;
       string r;
       string c;
 
       for(const auto& da : j)
       {   
           s = da["source"];
           d = da["datahash"];
           t = da["target"];
           u = da["user"];
           n = da["n"];
           r = da["rotation"];
           c = da["cost"];
       }
 
       beforeCost = s + d + t + u + n;
       finalCost = sha256(beforeCost);
 
       string current;
       string child = s;
       string target;
       string data;
 
       while (current != "NULL")
       {
           current = childToParent[child];
 
           if(childToParent[current] == "NULL")
           {
               current = child;
               break;
           }
          
           else
               child = current;
       }
 
       fstream file;
       file.open("jsonfile/" + current + ".json");
       json File = json::parse(file);
       file.close();
 
       for(const auto& obj : File)
       {
           target = obj["target"];
           data = obj["data"];
       }
 
       if(equal(target.begin(), target.begin() + std::min(target.size(), finalCost.size() ), finalCost.begin()))
       {
           cout << data << "\n";
           res.set_content(data, "text/plain");
       }
       else
       {
           cout << "invalid data" << "\n";
           res.set_content("Invalid!", "text/plain");
       }
 
   });
 
   svr.Post("/api/v2/job", [&](const Request &req, Response &res, const ContentReader &content_reader) {
       res.set_header("Access-Control-Allow-Origin", "*");
       // std::string source;
       // content_reader([&](const char *data, size_t data_length) {
       //     source.append(data, data_length);
       //     return true;
       // });
           cout << "svr" << "\n";
           struct Block Mine;
           int priority;
 
           if (req.has_param("source"))
           {
               if(req.get_param_value("source").compare(0, 5, "https") != 0)
               {
                   cout << "found source" << "\n";
                   Mine.source = req.get_param_value("source");
 
                   Mine.data = "";
                   Mine.target = "";
                   Mine.iteration = "1";
                  
                   if (req.has_param("data")) {
                       Mine.data = req.get_param_value("data");
                   }
 
                   if (req.has_param("target")) {
                       Mine.target = req.get_param_value("target");
                   }
 
                   if (req.has_param("iteration")) {
                       Mine.iteration = req.get_param_value("iteration");
                   }
 
                   if ((Mine.data == "") || (Mine.target == ""))
                   {
                       string parent = childToParent[Mine.source];
 
                       fstream file;
                       file.open("jsonfile/" + parent + ".json");
                       json File = json::parse(file);
                       file.close();
 
                       json jj = File[Mine.source];
 
                       Mine.data = jj["data"];
                       Mine.target = jj["target"];
                   }
 
                   ToMine(Mine);
               }
               else
               {
                   Mine.source = req.get_param_value("source");
                   Mine.data = req.get_param_value("data");
                   Mine.target = "21e8";
                   Mine.iteration = "1";
                  
                   ToMine(Mine);
               }
           }
           else
           {
               cout << "no url" << "\n";
               string body;
               content_reader([&](const char *data, size_t data_length) {
                   body.append(data, data_length);
                   return true;
               });
 
               json j = json::parse(body);
               // test
               cout << j.dump(4);
 
               Mine.source = j["source"];
               Mine.data = j["data"];
               Mine.target = j["target"];
               Mine.iteration = std::to_string((int)j["iteration"]);
 
               ToMine(Mine);
           } 
      
   });
 
   svr.listen("0.0.0.0", 2180);
 
   return 0;
 
}
 
void ToMine(Block Mine)
{
   // create a new hashing object
   SHA256 sha256;
 
   Trie insert;
 
   int numIterations = stoi(Mine.iteration);
 
   std::string sourceHash;
 
   if(size(Mine.source) == 64)
   {
       sourceHash = Mine.source;
   }
   else
   {
       sourceHash = sha256(Mine.source);
   }
 
   std::string dataHash;
 
   if(size(Mine.data) == 64)
   {
       dataHash = Mine.data;
   }
   else
   {
       dataHash = sha256(Mine.data);
   }
 
   std::string user = "aaaa";
   // std::string originalSource = Mine.source;
   uint64_t nonce = 0;
   std::string parent = "NULL";
 
   std::string SH = sourceHash;
 
   std::string beforeHash;
   std::string hash = "ffffffffffffffffffffffffffffffff";
   std::time_t timestamp = std::time(0);
 
   ofstream dataFile("data/" + dataHash + ".txt");
   dataFile << Mine.data << endl;
   dataFile.close();
 
   for (int i = 0; i < numIterations; i++)
   {
       // while (hash.compare(0, Mine.target.size(), Mine.target) != 0)
       // {
       //     ++nonce;
       //     beforeHash = sourceHash + dataHash + Mine.target + user + std::to_string(nonce);
       //     hash = sha256(beforeHash);
       // }
 
       // calling taskflow instead of above loop...
       nonce = Taskflow(sourceHash, dataHash, user, Mine.target);
       beforeHash = sourceHash + dataHash + Mine.target + user + std::to_string(nonce);
       hash = sha256(beforeHash);
 
       std::cout << "Before hash: " + beforeHash + "\n";
       std::cout << "Hash: " + hash + "\n";
       std::cout << "Nonce: " + std::to_string(nonce) + "\n";
 
       if(filesystem::exists("jsonfile/" + sourceHash + ".json"))
       {
           fstream file;
           file.open("jsonfile/" + sourceHash + ".json");
           json jo = json::parse(file);
           file.close();
          
           ofstream File("jsonfile/" + sourceHash + ".json");
 
           jo[hash] = {
               {"data", Mine.data},
               {"datahash", dataHash},
               {"n", to_string(nonce)},
               {"rotation", hash},
               {"source", sourceHash},
               {"target", Mine.target},
               {"cost", Mine.target},
               {"timestamp", timestamp},
               {"user", user}
           };
 
           File << jo.dump(4);
           File.close();
       }
       else
       {
           std::ofstream file("jsonfile/" + sourceHash + ".json");
           json j;
           j[hash] = {
               {"data", Mine.data},
               {"datahash", dataHash},
               {"n", to_string(nonce)},
               {"rotation", hash},
               {"source", sourceHash},
               {"target", Mine.target},
               {"cost", Mine.target},
               {"timestamp", timestamp},
               {"user", user}          
           };
 
           file << j.dump(4);
           file.close();
       }
 
       Insert(sourceHash, hash, parent);
       parent = sourceHash;
       //insert.InsertChars(hash);
 
       random_device nonce;
       default_random_engine generator(nonce());
       sourceHash = hash;
   }
  
   //PrintTables();
   MakeFiles();
   //RebuildMaps();
   //DepthFirstSearch(SH);
}