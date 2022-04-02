contract C {
    receive() external payable { }
    receive() external payable { }
    receive() external payable { }
}
// ----
// DeclarationError 4046: (52-82='receive() external payable { }'): Only one receive function is allowed.
// DeclarationError 4046: (87-117='receive() external payable { }'): Only one receive function is allowed.
