# rshttp

Small http server library

```cpp
#include "rshttp/rshttp.hpp"

int main() {
   rshttp::Server svr(8000)

   svr.route("/", [&](SOCKET cliSocket){
      svr.returnContent(cliSocket, svr.getFile("index.html", {
         {"{variable}", "replacement"}
      }), "text/html"); 
   });
   svr.route("/style.css", [&](SOCKET cliSocket){
      svr.returnContent(cliSocket, svr.getFile("style.css", {}), "text/css");  
   });

   svr.run();
   return 0;
}
