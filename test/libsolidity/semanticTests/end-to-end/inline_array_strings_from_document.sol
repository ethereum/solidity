contract C {
    function f(uint256 i) public returns (string memory) {
        string[4] memory x = ["This", "is", "an", "array"];
        return (x[i]);
    }
}

// ----
// f(uint256): 0 -> 0x20, 0x4, "This"
// f(uint256): 1 -> 0x20, 0x2, "is"
// f(uint256): 2 -> 0x20, 0x2, "an"
// f(uint256): 3 -> 0x20, 0x5, "array"
