Two Cache Line Ping Pong
========================

> This is a bare test, which means it doesn't use the fipc library.

In this test, we time a ping pong transaction between two processors using two cache lines.

### Test Procedures
0. Setup a requester and responder thread on separate processors, Px and Py.
1. Requester writes a request on cache line 1.
2. Responder reads the request on cache line 1.
3. Responder writes the response on cache line 2.
4. Requester reads the response on cache line 2.
 
```text
                                   +
      Requester (Px)               |    Responder (Py)
                                   |
+------------------------------------------------------------------------+
                                   |
 +------------------------+        |
 |   Line 1               |        |
 |   (1) Write Request,   |        |
 |       Store on Shared  |        |
 +------------------------+        |
                                   |
                                   |   +------------------------+
                                   |   |   Line 1               |
                                   |   |   (2) Read Request,    |
                                   |   |       Load on Modified |
                                   |   +------------------------+
                                   |
                                   |
                                   |   +------------------------+
                                   |   |   Line 2               |
                                   |   |   (3) Write Response,  |
                                   |   |       Store on Shared  |
                                   |   +------------------------+
                                   |
 +------------------------+        |
 |   Line 2               |        |
 |   (4) Read Response,   |        |
 |       Load on Modified |        |
 +------------------------+        |
                                   |
                                   |
                                   |
                                   +
```

### Cache Transactions
> Analyzed using the MESI protocol

**First transaction:**
* Step 1: Px get line 1 from MEM, write request, put into M
* Step 2: Py get line 1 from Px, read request, put into S
* Step 3: Py get line 2 from MEM, write response, put into M
* Step 4: Px get line 2 from Py, read response, put into S

**Every other transaction:**
* Step 1: Px get line 1 from Py, write request, put into M (store on shared)
* Step 2: Py get line 1 from Px, read request, put into S (load on modified)
* Step 3: Py get line 2 from Px, write response, put into M (store on shared)
* Step 4: Px get line 2 from Py, read response, put into S (load on modified)

> You can use [ccbench](https://github.com/trigonak/ccbench) to estimate the time these cache transactions should take. To test load on modified times run `ccbench -t7 -x# -y# -e2 -s1`, where `#` is replaced with the processor ids of the processors being tested. To test store on shared times run `ccbench -t3 -x# -y# -e2 -s1`.

    
