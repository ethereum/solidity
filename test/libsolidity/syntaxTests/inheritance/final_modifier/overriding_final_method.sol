contract Base {
    function f() public pure final {}
}
contract Derived is Base {
    function f() public pure {}
}
// ----
// TypeError: (87-114): Cannot override final function.
