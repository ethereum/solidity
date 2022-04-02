contract Test {
    function creation() public pure returns (bytes memory) {
        type();
    }
}
// ----
// TypeError 8885: (85-91='type()'): This function takes one argument, but 0 were provided.
