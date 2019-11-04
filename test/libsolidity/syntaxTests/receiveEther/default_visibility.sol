contract C {
    // Check that visibility is also enforced for the receive ether function.
    receive() {}
}
// ----
// SyntaxError: (95-107): No visibility specified. Did you intend to add "external"?
// TypeError: (95-107): Receive ether function must be payable, but is "nonpayable".
// TypeError: (95-107): Receive ether function must be defined as "external".
