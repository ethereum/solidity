contract Foo {
    function f(uint[] storage memory constant x, uint[] memory calldata y) internal { }
    function f2(uint[] storage storage x) internal { }
    function f3(uint[] storage calldata x) internal { }
    function f4(uint[] memory storage x) internal { }
    function f5(uint[] memory memory x) internal { }
    function f6(uint[] calldata storage x) internal { }
    function f7(uint[] calldata memory x) internal { }
    function f8(uint[] calldata calldata x) internal { }
}
// ----
// ParserError 3548: (45-51): Location already specified.
// ParserError 3548: (78-86): Location already specified.
// ParserError 3548: (134-141): Location already specified.
// ParserError 3548: (189-197): Location already specified.
// ParserError 3548: (244-251): Location already specified.
// ParserError 3548: (298-304): Location already specified.
// ParserError 3548: (353-360): Location already specified.
// ParserError 3548: (409-415): Location already specified.
// ParserError 3548: (464-472): Location already specified.
