function uintUintSuffix(uint x) pure suffix returns (uint, uint) { return (x, x); }

contract C {
    function g(uint, uint) public {}

    function f() public {
        abi.encode(1 uintUintSuffix);
        abi.encodePacked(2 uintUintSuffix);
        abi.encodeWithSelector(0x12345678, 3 uintUintSuffix);
        abi.encodeWithSignature("f()", 4 uintUintSuffix);
        abi.encodeCall(this.g, 5 uintUintSuffix);
    }
}
// ----
// TypeError 7848: (52-64): Literal suffix functions must return exactly one value.
// TypeError 2056: (181-197): This type cannot be encoded.
// TypeError 2056: (225-241): This type cannot be encoded.
// TypeError 2056: (287-303): This type cannot be encoded.
// TypeError 2056: (345-361): This type cannot be encoded.
// TypeError 9062: (395-411): Expected an inline tuple, not an expression of a tuple type.
