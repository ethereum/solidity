contract C {
    function f() public pure {
        bytes32 h = keccak256(keccak256, f, this.f.gas, block.blockhash);
        h;
    }
}
// ----
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
