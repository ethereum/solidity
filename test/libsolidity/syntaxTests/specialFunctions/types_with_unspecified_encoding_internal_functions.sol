contract C {
    function f() public pure {
        bytes32 h = keccak256(abi.encodePacked(keccak256, f, this.f{gas: 2}, blockhash));
        h;
    }
}
// ----
// TypeError 2056: (91-100): This type cannot be encoded.
// TypeError 2056: (102-103): This type cannot be encoded.
// TypeError 2056: (105-119): This type cannot be encoded.
// TypeError 2056: (121-130): This type cannot be encoded.
