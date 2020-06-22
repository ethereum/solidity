contract C {
    bytes32 constant x = keccak256("abc");
    bytes32 constant y = x;
    function f() public pure returns (uint t) {
        assembly {
            t := y
        }
    }
}
// ----
// TypeError 7615: (168-169): Only direct number constants and references to such constants are supported by inline assembly.
