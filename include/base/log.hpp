#pragma once  
#include <cstdio>  
#include <string>  
#include <map>  
#include <memory>

class Log {  
private:  
   std::shared_ptr<FILE> log_file;  

   enum class LogLevel : int8_t {  
       DEBUG = 0,  
       INFO,  
       WARN,  
       ERR, // Renamed from ERROR to ERR to avoid conflict with macros or reserved keywords  
       CRITICAL  
   };  

   void logger(LogLevel log_level, const std::string& msg);  
   std::string get_log_level_name(LogLevel log_level);  
   std::string get_time();  
   std::string log_name;  

   static std::map<std::string, Log> logs;  
public:  
   void debug(const std::string& msg);  
   void info(const std::string& msg);  
   void warn(const std::string& msg);  
   void error(const std::string& msg);  
   void critical(const std::string& msg);  
   void flush();  
   LogLevel log_level;  

   Log(const std::string& log_name, const std::string& file_name);  
   Log();  
   ~Log();  
   Log(const Log&) = delete;
   static Log* get_logger(const std::string& log_name, const std::string& file_name);  
   static void remove_logger(const std::string& log_name);

};
