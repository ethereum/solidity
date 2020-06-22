contract C {
    // Check that visibility is also enforced for the receive ether function.
    receive() {}
}
// ----
// SyntaxError 4937: (95-107): No visibility specified. Did you intend to add "external"?
// TypeError 7793: (95-107): Receive ether function must be payable, but is "nonpayable".
// TypeError 4095: (95-107): Receive ether function must be defined as "external".
