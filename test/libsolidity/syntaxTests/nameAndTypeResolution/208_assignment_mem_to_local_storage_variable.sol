contract C {
    uint[] data;
    function f(uint[] memory x) public {
        uint[] storage dataRef = data;
        dataRef = x;
    }
}
// ----
// TypeError: (128-129): Type uint256[] memory is not implicitly convertible to expected type uint256[] storage pointer.
