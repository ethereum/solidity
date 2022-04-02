contract C {
    function f() public pure {
        bytes32 h = keccak256(abi.encodePacked(keccak256, f, this.f{gas: 2}, blockhash));
        h;
    }
}
// ----
// TypeError 2056: (91-100='keccak256'): This type cannot be encoded.
// TypeError 2056: (102-103='f'): This type cannot be encoded.
// TypeError 2056: (105-119='this.f{gas: 2}'): This type cannot be encoded.
// TypeError 2056: (121-130='blockhash'): This type cannot be encoded.
