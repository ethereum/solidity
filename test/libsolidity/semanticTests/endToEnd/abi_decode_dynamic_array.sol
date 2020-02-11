contract C {
    function f(bytes calldata data) external pure returns(uint[] memory) {
        return abi.decode(data, (uint[]));
    }
}

// ----
// f(bytes): 32, 192, 32, 4, 3, 4, 5, 6 -> 32, 4, 3, 4, 5, 6
