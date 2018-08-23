# client-server Implementation

I try to compare with each IPC implementation, such as PIPE, threads, FIFO, Message Queue, socket, to implement client-server model.

## Problem settings

+ Each Client requests Server.

```
ClientA  -|
ClientB  -|-> Server
```

+ Server receives for each client as follows:

```
ClientA <--- Server for A <-| <fork>
ClientB <--- Server for B <-| 
                            |-Server
```

+ Then each client output results in STDOUT.

## Execution

```bash
./server 
./client "fileA" "fileB"
./client "fileC"
```

## Progress Rate


+ [x] pipe
+ [x] popen
+ [] FIFO
+ [] MultiThread
