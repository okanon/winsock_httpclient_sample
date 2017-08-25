# winsock_httpclient_sample
Sample of httpclient using C winsock

## How to Use / Build
It can be built with Visual Studio. To build using gcc, see `ws2_32.lib`. 
```
gcc -o httpclient -lwsock32 client.c -lws2_32
```
or 
```
make
```
How to use,
```
httpclient google.co.jp
```
Enter the host name you want to access as above.

That's all.


## License
This library is under [the MIT License (MIT)](LICENSE).