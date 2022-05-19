contract C {
    function f(bytes calldata x) public returns (bytes memory) {
        assembly { x.offset := 1 x.length := 3 }
        return x;
    }
}
// ----
// f(bytes): 0x20, 0, 0 -> 0x20, 3, 0x5754f80000000000000000000000000000000000000000000000000000000000
