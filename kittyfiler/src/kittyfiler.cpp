#include "connection.hpp"
#include "database.hpp"
#include "handler.hpp"
#include "cli.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <poll.h>
#include <signal.h>

int main(int argc, char** argv)
{
  int exitStatus = 0;
  // Use signal to setup handling of signals
  Handler::_outptr = &std::cerr;
  signal(SIGHUP, Handler::signalTerminate);
  signal(SIGINT, Handler::signalBreak);
  signal(SIGTERM, Handler::signalTerminate);
  signal(SIGABRT, Handler::signalTerminate);

  try
    {
      // Parse CLI arguments
      char oaList[] = {'f','h','d','u','P'};
      Cli::Args al(argc, argv, oaList, sizeof(oaList)/sizeof(oaList[0]));

      // If there are arguments given, try to log data
      if (al.size() == 1 && !Handler::terminateP())
	{
	  std::string special = al.arg(0);
	  Filer::Connection* c = NULL;
	  try
	    {
	      c = new Filer::Connection(special.c_str());
	      std::stringstream ss;

	      struct pollfd pfd;
	      pfd.fd = c->fd();
	      pfd.events = POLLIN;

	      long pollStat = 0;
	      while (pollStat >= 0 && !Handler::breakS())
		{
		  pollStat = poll(&pfd, 1, 60000);
		  if (pollStat > 0)
		    {
		      c->readUntil(ss, '\n');
		      if (al.option('p'))
			std::cout << ss.str() << std::endl;
		      if (al.option('f'))
			{
			  std::ofstream ofile;
			  ofile.open(al.optarg('f').c_str(),
				     std::ofstream::out | std::ofstream::app);
			  Filer::jsonToCSV(ss, ofile);
			}
		      if (al.option('b'))
			{
			  std::stringstream sparsed;
			  Filer::auth a;
			  if (al.option('d')) a.database = al.optarg('d');
			  if (al.option('h')) a.host = al.optarg('h');
			  if (al.option('u')) a.user = al.optarg('u');
			  if (al.option('P')) a.password = al.optarg('P');
			  ss.seekg(0);
			  Filer::jsonToCSV(ss, sparsed);
			  Filer::Database db(&a);
			  std::string tablename = "kittyfiler.ammonia";
			  if (!db.tableExists(tablename))
			    db.createTable(tablename,
					   {std::string("sentmillis"),
					    std::string("timemillis"),
					    std::string("value"),
					    std::string("warmedup")},
					   {std::string("integer"),
					    std::string("integer"),
					    std::string("numeric"),
					    std::string("bool")});
			  db.append(tablename, sparsed);
			}
		      std::stringstream tss;
		      ss.swap(tss);
		    }
		}
	    }
	  catch (std::exception& e)
	    {
	      if (c) c->closePort();
	      std::cout << e.what() << std::endl;
	      exitStatus = -1;
	    }
	}
    }
  catch (std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      exitStatus = -1;
    }

  std::cout << "Exiting" << std::endl;
  return exitStatus;
}
