contract C {
    uint x;
    receive() external pure { x = 2; }
}
// ----
// TypeError: (29-63): Receive ether function must be payable, but is "pure".
