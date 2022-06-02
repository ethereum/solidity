function nullSuffix(uint) pure {}

contract C {
    function g() public {}

    function h() public returns (uint, uint) {}

    event E();
    function f() public {
        abi.encode(1 nullSuffix);
        abi.encodePacked(2 nullSuffix);
        abi.encodeWithSelector(0x12345678, 3 nullSuffix);
        abi.encodeWithSignature("f()", 4 nullSuffix);
        //abi.encodeCall(this.g, 5 nullSuffix); // TODO: Causes an ICE
    }
}
// ----
// TypeError 2056: (185-197): This type cannot be encoded.
// TypeError 2056: (225-237): This type cannot be encoded.
// TypeError 2056: (283-295): This type cannot be encoded.
// TypeError 2056: (337-349): This type cannot be encoded.
