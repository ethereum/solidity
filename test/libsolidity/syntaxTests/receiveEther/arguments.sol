contract C {
    receive(uint256) external payable {}
}
// ----
// DeclarationError 6857: (24-33='(uint256)'): Receive ether function cannot take parameters.
