contract A {
    function x() public {}
}

contract B is A {
    function f() public view {
        A.x();
    }
}
// ----
// TypeError: (100-105): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
