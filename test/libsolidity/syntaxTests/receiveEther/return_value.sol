contract C {
    receive() external returns (uint256) {}
}
// ----
// DeclarationError 7793: (17-56): Receive ether function must be payable, but is "nonpayable".
// DeclarationError 6899: (44-53): Receive ether function cannot return values.
