contract C {
    function blobhash(uint256 index) public pure returns(bytes32) {
        return bytes32(index);
    }
    function f() public pure returns(bytes32) {
        return blobhash(3);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0x03
