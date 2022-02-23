contract C {
    function f(bytes memory data) public pure returns (uint256) {
        return abi.decode(data, (uint256));
    }
}

// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0x20, 0x21 -> 33
