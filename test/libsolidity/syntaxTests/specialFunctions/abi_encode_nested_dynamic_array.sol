contract C {
    function test() public pure {
        abi.encode([new uint[](5), new uint[](7)]);
    }
}
// ----
// TypeError 2056: (66-96): This type cannot be encoded.
