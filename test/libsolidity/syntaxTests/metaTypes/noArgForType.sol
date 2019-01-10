contract Test {
    function creation() public pure returns (bytes memory) {
        type();
    }
}
// ----
// TypeError: (85-91): This function takes one argument, but 0 were provided.
