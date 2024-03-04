contract C {
    function blobhash(uint256 index) public pure returns(bytes32) {
        return bytes32(index);
    }
    function f() public pure returns(bytes32) {
        return blobhash(2);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// Warning 2319: (17-117): This declaration shadows a builtin symbol.
