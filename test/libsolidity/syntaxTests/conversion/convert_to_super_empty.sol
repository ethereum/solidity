contract C {
    function f() public pure {
        super().x;
    }
}
// ----
// TypeError 1744: (52-59): Cannot convert to the super type.
