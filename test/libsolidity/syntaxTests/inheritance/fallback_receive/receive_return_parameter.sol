contract C {
    receive() external payable virtual returns(uint) {}
}
contract D is C {
    receive() external payable override {}
}
// ----
// DeclarationError 6899: (59-65): Receive ether function cannot return values.
