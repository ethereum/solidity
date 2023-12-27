contract C {
    function f(uint _index) public returns (bytes32) {
        return blobhash(_index);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f(uint256): 0 -> 0x0100000000000000000000000000000000000000000000000000000000000001
// f(uint256): 1 -> 0x0100000000000000000000000000000000000000000000000000000000000002
// f(uint256): 2 -> 0x00
// f(uint256): 255 -> 0x00
// f(uint256): 256 -> 0x00
// f(uint256): 257 -> 0x00
