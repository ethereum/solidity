contract Foo {
    function f(uint[] storage memory constant x, uint[] memory calldata y) internal { }
}
// ----
// ParserError 3548: (45-51): Location already specified.
// ParserError 3548: (78-86): Location already specified.
