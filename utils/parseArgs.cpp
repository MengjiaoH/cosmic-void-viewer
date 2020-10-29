#include "parseArgs.h"

std::string getFileExt(const std::string& s) 
{

   size_t i = s.rfind('.', s.length());
   if (i != std::string::npos) {
      return(s.substr(i+1, s.length() - i));
   }

   return("");
}

void parseArgs(int argc, const char **argv, Args &args)
{
    for(int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];
        if(arg == "-f"){
            args.filename = argv[++i];
        }else if(arg == "-dims"){
            args.dims.x = std::stoi(argv[++i]);
            args.dims.y = std::stoi(argv[++i]);
            args.dims.z = std::stoi(argv[++i]);
        }else if(arg == "-dtype"){
            args.dtype = argv[++i];
        }
    }
    // find file extension
    std::string extension = getFileExt(args.filename);
    args.extension = extension;
}