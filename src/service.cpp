/**                           _            _
 *        /\  /\___ _ __ ___ | | ___   ___| | __
 *       / /_/ / _ \ '_ ` _ \| |/ _ \ / __| |/ /
 *      / __  /  __/ | | | | | | (_) | (__|   <
 *      \/ /_/ \___|_| |_| |_|_|\___/ \___|_|\_\
 *      Tyler Neely 2015 t@jujit.su
 */
#include "proxy.hpp"
#include "storage.hpp"
#include "monitor.hpp"

namespace Hemlock {

/**
 *  A Service is composed of:
 *    1. Coordinator - joins shards and keeps metadata current
 *    2. Proxy - routes requests to the leader of a shard
 *    3. Storage - replication, splitting and persistence
 */
class Service {
public:
  Service() {}
  virtual ~Service() {}
private:
  Monitor monitor;
  Proxy proxy;
  Storage storage;
};

} // namespace Hemlock
