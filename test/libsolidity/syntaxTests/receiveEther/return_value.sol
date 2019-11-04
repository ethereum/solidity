contract C {
    receive() external returns (uint256) {}
}
// ----
// TypeError: (17-56): Receive ether function must be payable, but is "nonpayable".
// TypeError: (44-53): Receive ether function cannot return values.
