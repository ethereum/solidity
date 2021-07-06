contract C {
    uint x;
    receive() external view { x = 2; }
}
// ----
// DeclarationError 7793: (29-63): Receive ether function must be payable, but is "view".
