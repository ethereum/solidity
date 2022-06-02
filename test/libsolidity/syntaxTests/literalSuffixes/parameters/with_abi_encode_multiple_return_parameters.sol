function uintUintSuffix(uint x) pure returns (uint, uint) { return (x, x); }

contract C {
    function g(uint, uint) public {}

    function f() public {
        abi.encode(1 uintUintSuffix);
        abi.encodePacked(2 uintUintSuffix);
        abi.encodeWithSelector(0x12345678, 3 uintUintSuffix);
        abi.encodeWithSignature("f()", 4 uintUintSuffix);
        //abi.encodeCall(this.g, 5 uintUintSuffix); // TODO: Causes an ICE
    }
}
// ----
// TypeError 2056: (174-190): This type cannot be encoded.
// TypeError 2056: (218-234): This type cannot be encoded.
// TypeError 2056: (280-296): This type cannot be encoded.
// TypeError 2056: (338-354): This type cannot be encoded.
