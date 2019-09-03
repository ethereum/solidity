contract C {
    function f(uint256[42] memory x) public pure {
        x[1:2];
    }
}
// ----
// TypeError: (72-78): Index range access is only supported for dynamic calldata arrays.
