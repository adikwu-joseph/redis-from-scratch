
# Redis-Style Server in C++
A Redis-inspired in-memory data store built from scratch in C++ using low-level system programming techniques.
This project implements a fully functional event-driven server, including custom networking, data structures, memory management, and background processing — without relying on external frameworks.
## What Makes This Different
This is not a wrapper around existing libraries. Every core component is implemented manually:
- Custom hash table with incremental rehashing
- AVL-tree-backed sorted sets (instead of skip lists)
- Custom binary protocol (not RESP)
- Event-driven networking using `poll()`
- TTL system using a min-heap
- Idle connection tracking via a doubly linked list
- Background cleanup using a thread pool
## Architecture
### Event Loop
- Single-threaded event loop using `poll()`
- Non-blocking sockets for all connections
- Supports request pipelining (multiple requests per read)
- Read/write intent is explicitly managed per connection
Each connection maintains:
- Input/output buffers
- Activity timestamps
- Read/write state flags
### Custom Protocol
Requests use a length-prefixed format:

+——+—–+——+—–+——+—–+
| nstr | len | str1 | len | str2 | … |
+——+—–+——+—–+——+—–+

Responses use a tagged binary format:
- `TAG_NIL`, `TAG_STR`, `TAG_INT`, `TAG_DBL`, `TAG_ARR`, `TAG_ERR`
This avoids parsing overhead and keeps serialization predictable.
### Data Store
The database is backed by a custom hash map:
```cpp
struct Entry {
    std::string key;
    uint32_t type;
    std::string str;
    ZSet zset;
};

Supported types:

* String
* Sorted Set (ZSet)

Hash Table (Core Detail)

* Separate chaining using linked nodes
* Two-table system (newer + older)
* Incremental rehashing:
    * Keys are gradually migrated during operations
    * Avoids latency spikes from full-table rehash

Sorted Sets (ZSet)

Unlike Redis (which uses skip lists), this implementation uses:

* AVL Tree → maintains sorted order (score, name)
* Hash Table → O(1) lookup by key

Key design details:

* Ordering is based on (score, name) tuple
* Updates require:
    * Remove from AVL tree
    * Reinsert with new score
* Range queries implemented via:
    * zset_seekge (lower bound search)
    * znode_offset (relative traversal)

TTL / Expiration

* Managed using a min-heap
* Each entry stores a reference to its heap index
* Expiration is processed incrementally:

while (!heap.empty() && heap[0].val < now_ms)

* Limits work per loop to prevent blocking (k_max_works)

Idle Connection Handling

* Connections stored in a doubly linked list
* Oldest connections checked first
* Idle timeout enforced without scanning all connections

Memory & Cleanup Strategy

* Large structures (e.g. big ZSets) are freed asynchronously
* Uses a thread pool to avoid blocking the main loop

if (set_size > threshold)
    thread_pool_queue(...)

Small objects are freed synchronously to avoid overhead.

Supported Commands

Key-Value

Command	Description
get key	Retrieve value
set key value	Set value
del key	Delete key
keys	List all keys

Expiration

Command	Description
pexpire key ms	Set TTL
pttl key	Get remaining TTL

Sorted Sets

Command	Description
zadd zset score name	Add/update element
zrem zset name	Remove element
zscore zset name	Get score
zquery zset score name offset limit	Range query

Build

g++ -o server src/server.cpp src/hashtable.cpp src/avl.cpp \
    src/zset.cpp src/heap.cpp src/thread_pool.cpp
g++ -o client src/client.cpp

Run

./server
./client set key value
./client get key
./client del key

Key Design Decisions

* AVL Tree instead of Skip List
    * Deterministic balancing
    * Simpler to reason about in a learning context
* Incremental Rehashing
    * Prevents blocking during resizing
    * Distributes work across operations
* Min-Heap for TTL
    * Efficient expiration (O(log n))
    * No full scans required
* Single-threaded I/O + Thread Pool
    * Avoids locking complexity in networking
    * Offloads expensive cleanup work

Limitations

* No persistence (in-memory only)
* Custom protocol (not Redis-compatible)
* No replication or clustering

Future Improvements

* RESP protocol support (Redis compatibility)
* Persistence (RDB / AOF)
* Performance benchmarking
* Memory profiling and optimisation
* Replace AVL with skip list for comparison

Reference

Inspired by: https://build-your-own.org/redis/

