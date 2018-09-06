contract Foo {
    function f(uint[] storage memory constant x, uint[] memory calldata y) internal { }
}
// ----
// ParserError: (45-51): Location already specified.
// ParserError: (78-86): Location already specified.
