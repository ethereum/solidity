contract C {
    function f(bytes calldata x) external pure returns (bytes memory) {
        return x[1:2];
    }
}