FIPC Ping Pong
========================

In this test, we time a ping pong transaction between two processors using the fipc mechanism.

### Test Procedures
0. Setup a requester and responder thread on separate processors, Px and Py, with an fipc channel.
1. Requester sends request message.
2. Responder reads request message.
3. Responder sends response.
4. Requester reads response.
