contract C {
    receive() external payable { msg.data; }
}
// ----
// TypeError 7139: (46-54): "msg.data" cannot be used inside of "receive" function.
