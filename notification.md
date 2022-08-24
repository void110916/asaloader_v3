# Notification

* [qt in vscode](https://www.kdab.com/using-visual-studio-code-for-writing-qt-applications/)
* [Converting from signed char to unsigned char and back again](https://stackoverflow.com/questions/5040920/converting-from-signed-char-to-unsigned-char-and-back-again)

* [Why doesn't reinterpret_cast convert 'unsigned char' to 'char'](https://stackoverflow.com/questions/14692418/why-doesnt-reinterpret-cast-convert-unsigned-char-to-char)

* [constexpr](https://tjsw.medium.com/%E6%BD%AE-c-constexpr-ac1bb2bdc5e2)

* [std::vector peformance](https://stackoverflow.com/questions/381621/using-arrays-or-stdvectors-in-c-whats-the-performance-gap#381656)

* [rvalue referance (&&) and std::move (move without copy)](https://tjsw.medium.com/%E6%BD%AE-c-11-universal-reference-rvalue-reference-move-semantics-1ea29f8cab.dc)

* [forward referance](https://tjsw.medium.com/%E6%BD%AE-c-11-perfect-forwarding-%E5%AE%8C%E7%BE%8E%E8%BD%89%E7%99%BC%E4%BD%A0%E7%9A%84%E9%9C%80%E6%B1%82-%E6%B7%B1%E5%BA%A6%E8%A7%A3%E6%9E%90-f991830bcd84)
  若是用樣板（template），須使用 forward referance 推導 (deduce)

* [range for](https://en.cppreference.com/w/cpp/language/range-for)

* [Inheriting using](https://en.cppreference.com/w/cpp/language/using_declaration)

* [std::vector move to std::vector **make_move_iterator**](https://en.cppreference.com/w/cpp/iterator/make_move_iterator)

## qt console

[教學](https://www.lubby.org/ebooks/qtconsoleapp2/qtconsoleapp2.html)

###　iostream

1. 用 `QTextStream` 包裝 stdout，若結尾用 `\n`非endl，則不會馬上更新(flush)
2. std::cout
3. qDebug，結尾會多一個換行
## qserialport

[DTR, RTS issue](https://forum.qt.io/topic/83679/problem-facing-with-qt-serial-port-module/5)