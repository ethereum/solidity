contract Test {
    function creation() public pure returns (bytes memory) {
        type(1, 2);
    }
}
// ----
// TypeError 8885: (85-95='type(1, 2)'): This function takes one argument, but 2 were provided.
