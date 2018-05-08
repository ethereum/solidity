contract C {
    function f() public pure returns(bytes1) {
        return bytes1(uint256(0));
    }
}
// ----
// TypeError: (75-93): Explicit type conversion not allowed from "uint256" to "bytes1".
