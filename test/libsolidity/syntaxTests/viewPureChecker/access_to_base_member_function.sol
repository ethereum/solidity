contract A {
    function x() public {}
}

contract B is A {
    function f() public view {
        A.x();
    }
}
// ----
// TypeError 8961: (100-105): Function cannot be declared as view because this expression (potentially) modifies the state.
