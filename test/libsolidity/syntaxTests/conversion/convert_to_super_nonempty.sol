contract C {
    function f() public pure {
        super(this).f();
    }
}
// ----
// TypeError 1744: (52-63='super(this)'): Cannot convert to the super type.
