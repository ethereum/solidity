contract B {
    uint immutable x;
}

contract C is B {
    function f() public {
        B.x = 42;
    }
}
// ----
// TypeError 1581: (90-93): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
