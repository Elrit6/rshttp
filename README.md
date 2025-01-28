# rshttp

Small http server library

An example program that'll add up and display two numbers given in parameters:
```cpp
#include "rshttp/rshttp.hpp"

int main() {
   rshttp::Server svr(8000);

   svr.route("/", [&](SOCKET cliSock, rshttp::Server::Req req, rshttp::Server::Res res){
      auto params = req.getParams();
      int a = std::stoi(params["a"]);
      int b = std::stoi(params["b"]);

      res.returnContent(svr.getFile("test.html", {
         {"{a}", std::to_string(a)},
         {"{b}", std::to_string(b)},
         {"{sum}", std::to_string(a + b)},
      }), "text/html");
   });

   svr.run();
   return 0;
}
