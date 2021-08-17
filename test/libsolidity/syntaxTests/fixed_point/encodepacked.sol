contract C {
    function f() public pure returns (bytes memory) {
        return abi.encode(
            1.23,
            1/3
        );
    }
}
// ----
// TypeError 8009: (124-127): Invalid rational number (too large or division by zero).
