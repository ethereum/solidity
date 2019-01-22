contract Test {
    function creation() public pure returns (bytes memory) {
        type(1, 2);
    }
}
// ----
// TypeError: (85-95): This function takes one argument, but 2 were provided.
