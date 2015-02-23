# [WIP] Hemlock
Distributed transactional kv store.  Mutant baby whose parents are Ceph, Calvin, Kafka and Mesos. Oh baby!

#### eventual architecture:
* high throughput distributed transactions
* serializable snapshot isolation for external consistency
* homogeneous services that compete for tunable quorum responsibilities
* load-based automatic resharding
* metadata (CRUSH map) distribution via gossip + polling hybrid
* backed by rocksdb instances with configurable replication factor
* implemented as a c++ mesos framework

#### ideas based on
* [Calvin](http://cs-www.cs.yale.edu/homes/dna/papers/calvin-sigmod12.pdf)
* [Ceph (CRUSH)](https://courses.cs.washington.edu/courses/cse444/08au/544M/READING-LIST/fekete-sigmod2008.pdf)
* [SWIM](http://www.cs.cornell.edu/~asdas/research/dsn02-swim.pdf)
* [Spanner](https://www.usenix.org/system/files/conference/osdi12/osdi12-final-16.pdf)
