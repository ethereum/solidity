contract C {
    uint[] data;
    function f(uint[] x) public {
        uint[] storage dataRef = data;
        dataRef = x;
    }
}
// ----
// TypeError: (121-122): Type uint256[] memory is not implicitly convertible to expected type uint256[] storage pointer.
