# port binder
## How to build

1. edit `config.h` file

2. Build server and client with `make build`


## Usage
1. start server
```
./server
```

2. start client
```
./client <source-port> <remote-port>
```

Or you can use `port.conf` file to specify the ports to bind.

```
./server -c port.conf
```