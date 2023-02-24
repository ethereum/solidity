function nullSuffix(uint) pure suffix {}

contract C {
    function g() public {}

    function h() public returns (uint, uint) {}

    event E();
    function f() public {
        abi.encode(1 nullSuffix);
        abi.encodePacked(2 nullSuffix);
        abi.encodeWithSelector(0x12345678, 3 nullSuffix);
        abi.encodeWithSignature("f()", 4 nullSuffix);
        abi.encodeCall(this.g, 5 nullSuffix);
    }
}
// ----
// TypeError 7848: (38-38): Literal suffix functions must return exactly one value.
// TypeError 2056: (192-204): This type cannot be encoded.
// TypeError 2056: (232-244): This type cannot be encoded.
// TypeError 2056: (290-302): This type cannot be encoded.
// TypeError 2056: (344-356): This type cannot be encoded.
// TypeError 9062: (390-402): Expected an inline tuple, not an expression of a tuple type.
