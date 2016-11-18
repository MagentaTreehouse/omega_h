#include "stacktrace.hpp"

#include <cxxabi.h>
#include <execinfo.h>
#include <cstdlib>
#include <sstream>
#include <string>

namespace Omega_h {

// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0

/* Dan Ibanez: found this code for stacktrace printing,
   cleaned it up for compilation as strict C++11,
   fixed the parser for OS X */

/** Print a demangled stack backtrace of the caller function to FILE* out. */

void print_stacktrace(std::ostream& out, int max_frames) {
  out << "stack trace:\n";
  // storage array for stack trace address data
  void** addrlist = new void*[max_frames];
  // retrieve current stack addresses
  int addrlen = backtrace(addrlist, max_frames);
  if (addrlen == 0) {
    out << "  <empty, possibly corrupt>\n";
    return;
  }
  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char** symbollist = backtrace_symbols(addrlist, addrlen);
  delete[] addrlist;
  // iterate over the returned symbol lines. skip the first, it is the
  // address of this function.
  for (int i = 1; i < addrlen; i++) {
#if defined(__APPLE__)
    std::string line(symbollist[i]);
    std::stringstream instream(line);
    std::string num;
    instream >> num;
    std::string obj;
    instream >> obj;
    obj.resize(20, ' ');
    std::string addr;
    instream >> addr;
    std::string symbol;
    instream >> symbol;
    std::string offset;
    instream >> offset >> offset;
    int status;
    char* demangled = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, &status);
    if (status == 0) {
      symbol = demangled;
    }
    free(demangled);
    out << "  " << num << ' ' << obj << ' ' << addr << ' ' << symbol << " + "
        << offset << '\n';
#elif defined(__linux__)
    std::string line(symbollist[i]);
    auto open_paren = line.find_first_of('(');
    auto start = open_paren + 1;
    auto plus = line.find_first_of('+');
    if (!(start < line.size() && start < plus && plus < line.size())) {
      out << symbollist[i] << '\n';
      continue;
    }
    auto symbol = line.substr(start, (plus - start));
    int status;
    char* demangled = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, &status);
    if (status == 0) {
      symbol = demangled;
    }
    free(demangled);
    out << line.substr(0, start) << symbol
        << line.substr(plus, line.size()) << '\n';
#else
    out << symbollist[i] << '\n';
    fprintf(out, "%s\n", symbollist[i]);
#endif
  }
  free(symbollist);
}

}  // end namespace Omega_h
