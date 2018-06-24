contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// TypeError: (30-55): Data location has to be "memory" (or unspecified) for constants.
