contract C {
    function f(bytes memory x) public pure {
        x[1:2];
    }
}
// ----
// TypeError: (66-72): Index range access is only supported for dynamic calldata arrays.
