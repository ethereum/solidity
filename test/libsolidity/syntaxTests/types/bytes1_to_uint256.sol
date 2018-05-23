contract C {
    function f() public pure returns(uint256) {
        return uint256(bytes1(''));
    }
}
// ----
// TypeError: (76-95): Explicit type conversion not allowed from "bytes1" to "uint256".
