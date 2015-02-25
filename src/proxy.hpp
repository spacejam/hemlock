/**                           _            _
 *        /\  /\___ _ __ ___ | | ___   ___| | __
 *       / /_/ / _ \ '_ ` _ \| |/ _ \ / __| |/ /
 *      / __  /  __/ | | | | | | (_) | (__|   <
 *      \/ /_/ \___|_| |_| |_|_|\___/ \___|_|\_\
 *      Tyler Neely 2015 t@jujit.su
 */

#include <process/future.hpp>
#include <string>

using std::string;

namespace Hemlock {

class Request {
private:

  string req;
  Future<string> rep;
}

//TODO(tan) decide on events vs handler thread stack
class Proxy {
public:
  Proxy() {}
  virtual ~Proxy() {}
private:
  Disruptor<Request> pending;
  map<string, Shard> shards;
};

} // namespace Hemlock
