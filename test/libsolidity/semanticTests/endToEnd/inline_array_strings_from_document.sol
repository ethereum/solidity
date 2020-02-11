contract C {
    function f(uint i) public returns(string memory) {
        string[4] memory x = ["This", "is", "an", "array"];
        return (x[i]);
    }
}

// ----
// f(uint256): 0) -> 0x20, 4, string("This"
// f(uint256):"0" -> "32, 4, This"
// f(uint256): 1) -> 0x20, 2, string("is"
// f(uint256):"1" -> "32, 2, is"
// f(uint256): 2) -> 0x20, 2, string("an"
// f(uint256):"2" -> "32, 2, an"
// f(uint256): 3) -> 0x20, 5, string("array"
// f(uint256):"3" -> "32, 5, array"
