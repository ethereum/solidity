contract C {
    function f() public returns (uint) { var x; return 2; }
}
// ----
// Warning: (54-59): Use of the "var" keyword is deprecated.
// TypeError: (54-59): Assignment necessary for type detection.
