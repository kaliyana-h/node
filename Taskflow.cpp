#include "executor.hpp"
#include "taskflow.hpp"
#include "observer.hpp"
#include "task.hpp"
#include "graph.hpp"
#include "iterator.hpp"
#include "sha256.cpp"
#include <taskflow.hpp>
#include <iostream>
#include <string>
 
void Taskflow(string sourceHash, string dataHash, string user) {
  
   tf::Taskflow taskflow;
   tf::Executor executor;
 
   SHA256 sha256;
 
   struct Block Mine;
 
   int hashNonce;
   int nonce;
   string beforeHash;
   string hash;
 
   auto init = taskflow.emplace([&]()
   {
       hashNonce = 0;
       random_device nonce;
       default_random_engine generator(nonce());
 
   }).name("init");
 
   auto do_work = taskflow.for_each_index(0, 8, 1, [&](int index)
   {
       // std::cout << "Index = " + std::to_string(index) + " i = " + std::to_string(i++) + "++." + "\n";
       for  (int i = 0; i < 1000; i++)
       {
           while (hash.compare(0, Mine.target.size(), Mine.target) != 0)
           {
               ++nonce;
               nonce = nonce + (index * 1000) + i;
               beforeHash = sourceHash + dataHash + Mine.target + user + std::to_string(nonce);
               hash = sha256(beforeHash);
           }
           if (hash.compare(0, Mine.target.size(), Mine.target) == 0)
           {
               hashNonce = nonce;
           }
       }
 
   }).name("do_work");
 
   auto check = taskflow.emplace([&]()
   {
       nonce += 8000;
       return hashNonce > 0;
 
   }).name("check");
 
   auto complete = taskflow.emplace([&]()
   {
       return hashNonce;
 
   }).name("complete");
 
   init.precede(do_work);
   do_work.precede(check);
   check.precede(do_work, complete);
 
   executor.run(taskflow).wait();
}