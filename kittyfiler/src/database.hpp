#include <iostream>
#include <vector>

#ifndef database_hpp
#define database_hpp

namespace Filer
{
  struct auth
  {
  public:
    std::string database = "postgres";
    std::string host = "localhost";
    std::string user = "postgres";
    std::string port = "5432";
    std::string password = "";
  };

  class Database
  {
  public:
    typedef std::vector<std::string> svector;
    Database();
    explicit Database(auth* a);
    void init(auth* a);
    void clear();
    int append(const std::string& table, std::istream& data);
    bool tableExists(const std::string& table);
    int createTable(std::string table, svector headers,
		    svector types);

  private:
    auth* _auth = NULL;
    std::string _conString();
  };
}

#endif
