//======================================================================
// A user friendly 3D model viewer with a qt interface.
//
// 2024-06-22 Sat
// Dov Grobgeld <dov.grobgeld@gmail.com>
//
// License:
//   This program is licensed under the GPL v2 license
//----------------------------------------------------------------------

#include "myapp.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include <string>
#include <fmt/core.h>
#include <QMessageBox>
#include "buildsha1.h"

using namespace std;

#define CASE(s) if (!strcmp(s, S_))

static string join(const vector<string>& v, const string& glue)
{
  string ret;
  for (int i=0; i<(int)v.size(); i++)
  {
    ret += v[i];
    if (i < (int)v.size()-1)
      ret += glue;
  }
  return ret;
}

int main(int argc, char *argv[])
{
    string log_filename;
    int argp = 1;
    bool do_debug = true;
    std::vector<std::string> args(argv, argv+argc);

    while(argp < argc && argv[argp][0] == '-') {
        char *S_ = argv[argp++];

        CASE("--help") {
            string HelpMessage
                = fmt::format(
                    "qtfern - A 3D viewer\n"
                    "\n"
                    "Syntax:\n"
                    "    qtfern [model]\n"
                    "\n"
                    "Options:\n"
                    "    --log_file log_file   Log debug info to the given file name\n"
                    "    --debug               Increase log level to debug\n"
                    );
#ifdef _WIN32
            QMessageBox::information (nullptr,
                                      "Command line Help",
                                      HelpMessage.c_str());
#else
            fmt::print("{}", HelpMessage);
#endif      
            exit(0);
        }

        CASE("--log_file")
        {
            log_filename = argv[argp++];
            continue;
        }
        CASE("--debug")
        {
            do_debug = true;
            continue;
        }
        // Currently ignore unknown options
    }

    MyApp app(argc, argv);

    vector<spdlog::sink_ptr> log_sinks;
    if (log_filename.size())
    {
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, 1024*1024*10, 3);
        log_sinks.push_back(rotating_sink);

    }
    auto log_level = spdlog::level::info;

    if (do_debug)
        log_level = spdlog::level::debug;

    auto logger = std::make_shared<spdlog::logger>("qtvsgviewer logger", log_sinks.begin(), log_sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_level(log_level);
    logger->set_pattern("[%H:%M:%S] [%l] %v");

    spdlog::info("======================================================");
    spdlog::info("Starting qtfern");
    spdlog::info("CommitID: {}", BUILD_SHA1);
    spdlog::info("CommitTime: {}", BUILD_COMMIT_TIME);
    spdlog::info("Command line: {}", join(args," "));


    app.exec();
    
    exit(0);
}
 
