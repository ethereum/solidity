contract C {
    function f(uint256[42] memory x) public pure {
        x[1:2];
    }
}
// ----
// TypeError 1227: (72-78='x[1:2]'): Index range access is only supported for dynamic calldata arrays.
