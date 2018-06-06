contract C {
    uint[] data;
    function f(uint[] x) public {
        var dataRef = data;
        dataRef = x;
    }
}
// ----
// Warning: (72-83): Use of the "var" keyword is deprecated.
// TypeError: (110-111): Type uint256[] memory is not implicitly convertible to expected type uint256[] storage pointer.
