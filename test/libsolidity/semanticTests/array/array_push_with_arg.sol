contract C {
    uint[] storageArray;
    function test(uint256 v) public {
        storageArray.push(v);
    }
    function getLength() public view returns (uint256) {
        return storageArray.length;
    }
    function fetch(uint256 a) public view returns (uint256) {
        return storageArray[a];
    }
}
// ----
// getLength() -> 0
// test(uint256): 42 ->
// getLength() -> 1
// fetch(uint256): 0 -> 42
// fetch(uint256): 1 -> FAILURE, hex"4e487b71", 0x32
// test(uint256): 23 ->
// getLength() -> 2
// fetch(uint256): 0 -> 42
// fetch(uint256): 1 -> 23
// fetch(uint256): 2 -> FAILURE, hex"4e487b71", 0x32
