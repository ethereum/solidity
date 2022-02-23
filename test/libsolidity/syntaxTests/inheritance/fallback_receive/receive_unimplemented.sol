interface I {
    receive(bytes2) external payable;
}

interface J is I {
    receive() external payable override;
}

contract C is J {
    receive() external payable override {}
}
// ----
// DeclarationError 6857: (25-33): Receive ether function cannot take parameters.
