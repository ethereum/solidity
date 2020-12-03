pragma abicoder v1;
contract C {
    function test() public pure {
        abi.encode([new uint[](5), new uint[](7)]);
    }
}
// ----
// TypeError 2056: (86-116): This type cannot be encoded.
