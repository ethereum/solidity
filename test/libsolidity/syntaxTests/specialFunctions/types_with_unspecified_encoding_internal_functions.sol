contract C {
    function f() public pure {
        bytes32 h = keccak256(keccak256, f, this.f.gas, block.blockhash);
        h;
    }
}
// ----
// TypeError: (74-83): This type cannot be encoded.
// TypeError: (85-86): This type cannot be encoded.
// TypeError: (88-98): This type cannot be encoded.
// TypeError: (100-115): This type cannot be encoded.
