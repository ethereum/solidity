error StringAndUint(string a, uint256 b);

contract C {
    function namedMisordered(string calldata s) external pure {
        require(false, StringAndUint({b: 42, a: s}));
    }
    function namedOrdered(string calldata s) external pure {
        require(false, StringAndUint({a: s, b: 42}));
    }
    function positional(string calldata s) external pure {
        require(false, StringAndUint(s, 42));
    }
}
// ----
// namedMisordered(string): 0x20, 5, "hello" -> FAILURE, hex"81a3bbac", 0x40, 42, 5, "hello"
// namedOrdered(string): 0x20, 5, "hello" -> FAILURE, hex"81a3bbac", 0x40, 42, 5, "hello"
// positional(string): 0x20, 5, "hello" -> FAILURE, hex"81a3bbac", 0x40, 42, 5, "hello"
