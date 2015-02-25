/**                           _            _
 *        /\  /\___ _ __ ___ | | ___   ___| | __
 *       / /_/ / _ \ '_ ` _ \| |/ _ \ / __| |/ /
 *      / __  /  __/ | | | | | | (_) | (__|   <
 *      \/ /_/ \___|_| |_| |_|_|\___/ \___|_|\_\
 *      Tyler Neely 2015 t@jujit.su
 */
#include <process/future.hpp>

#include <glog/logging.h>

#include <stout/os.hpp>

#include "rocksdb/db.h"

using std::string;
using process::Future;

class Storage {
public:
  //TODO(tan) This should not be something that can fail, so initialize outside.
  Storage()
  {
    options.create_if_missing = true;
    rocksdb::Status s = rocksdb::DB::Open(options, "rocksdb_storage", &db);
    if (s.ok()) {
      LOG(INFO) << "Initialized rocksdb in " << os::getcwd() << "/storage";
    } else {
      LOG(ERROR) << "Failed to initialize rocksdb: " << s.ToString();
    }
  }

  virtual Option<string> Get(string key)
  {
    std::string value;
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key, &value);
    if (s.ok()) {
      return Some(value);
    } else {
      return None();
    }
  }

  virtual Option<bool> Put(string key, string value)
  {
    rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
    if (s.ok()) {
      return Some(true);
    } else {
      LOG(ERROR) << "Failed to put key: " << s.ToString();
      return None();
    }
  }

  virtual Option<bool> Delete(string key)
  {
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), key);
    if (s.ok()) {
      return Some(true);
    } else {
      return None();
    }
  }
 
  virtual ~Storage()
  {
    delete db;
  }

private:
  rocksdb::Options options;
  rocksdb::DB* db;
};
