function uintSuffix(uint x) pure returns (uint) { return x; }

contract C {
    function g(uint) public {}

    function f() public {
        abi.encode(1 uintSuffix);
        abi.encodePacked(2 uintSuffix);
        abi.encodeWithSelector(0x12345678, 3 uintSuffix);
        abi.encodeWithSignature("f()", 4 uintSuffix);
        abi.encodeCall(this.g, 5 uintSuffix);
    }
}
// ----
// Warning 2018: (112-371): Function state mutability can be restricted to view
