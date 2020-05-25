#### http 1.1 server implementation
  
#### Benchmark
Note: benchmark was run within a Linux VM, and benchmark tool was run on the same machine
System specs:
- 3 core * 1.8 GHz
- 3 GB RAM
```
./wrk -t12 -c500 -d10s http://127.0.0.1:1337/echo
Running 10s test @ http://127.0.0.1:1337/echo
  12 threads and 500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    14.46ms   74.67ms   1.67s    97.01%
    Req/Sec     1.24k   642.26     6.49k    78.86%
  138748 requests in 10.09s, 12.44MB read
  Socket errors: connect 0, read 138748, write 0, timeout 0
Requests/sec:  13755.99
Transfer/sec:      1.23MB
```
  
#### Build and run examples/example1.cc
```bash
mkdir build;
cd build;
cmake ..
make
./example1
```

#### Missing features
- Understand of request headers:
    + Keep alive connection
    + Multipart data
    + Content encoding
    + ...
- Parse query string
- Server static files
- Routing with parameters on URI (eg. GET /userid/spham)