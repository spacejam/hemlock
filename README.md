# [WIP] Hemlock
Distributed transactional kv store.  Mutant baby whose parents are Ceph, Calvin, Kafka and Mesos. Oh baby!

#### eventual architecture:
* homogeneous services that compete for tunable quorum responsibilities
* load-based automatic resharding
* metadata (CRUSH map) distribution via gossip + polling hybrid
* backed by rocksdb instances with configurable replication factor
* implemented as a c++ mesos framework
* high throughput distributed transactions a la calvin
