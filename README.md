# ASALoader V3

## Used Library
* Qt: v6.3.1
* serial port: QserialPort
    **若要加 thread，須使用 Qthread 而非 pthread**
    
    網路上有人說qserialport很難用，建議用 boost::asio::serial_port
## Work Lists

- [X] Decoder *untest*

- [ ] CMD **--working**

- [ ] Loader

- [ ] test main

- [ ] error handle

- [ ] timeout exception
## Issue

### binary read/write problem
因指令有超過 `127(aka 0x7f)`，在比較時無法直接用 char 做比較，因此需要用 `reinterpret_cast` 將 char 直接用 unsigned char 的方式讀取，以防止強轉所造成的數值改變或是編譯時的錯誤。
#### reference
[Converting from signed char to unsigned char and back again](https://stackoverflow.com/questions/5040920/converting-from-signed-char-to-unsigned-char-and-back-again)

[Why doesn't reinterpret_cast convert 'unsigned char' to 'char'](https://stackoverflow.com/questions/14692418/why-doesnt-reinterpret-cast-convert-unsigned-char-to-char)
