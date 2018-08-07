contract Foo {
    function f(uint[] storage constant x, uint[] memory y) internal { }
}
// ----
// TypeError: (30-55): Location has to be memory for reference type constants. Remove the data location keyword to fix this error.
