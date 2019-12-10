contract C {
    function receive() external pure {}
}
// ----
// Warning: (26-33): This function is named "receive" but is not the receive function of the contract. If you intend this to be a receive function, use "receive(...) { ... }" without the "function" keyword to define it.
