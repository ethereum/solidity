contract C {
    function f() public {
        uint[] memory x;
        x.length = 2;
    }
}
// ----
// TypeError 7567: (72-80): Member "length" is read-only and cannot be used to resize arrays.
