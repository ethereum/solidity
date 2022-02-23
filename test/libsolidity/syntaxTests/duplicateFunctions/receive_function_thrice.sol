contract C {
    receive() external payable { }
    receive() external payable { }
    receive() external payable { }
}
// ----
// DeclarationError 4046: (52-82): Only one receive function is allowed.
// DeclarationError 4046: (87-117): Only one receive function is allowed.
