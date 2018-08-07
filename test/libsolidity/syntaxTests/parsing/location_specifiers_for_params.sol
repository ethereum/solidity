contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// TypeError: (30-55): Storage location must be none or "memory" for constant parameter in internal function, but "storage" was given.
