contract C {
    bytes32 constant H = keccak256({_data: "abc"});
    uint[uint(H)] arr;
}
// ----
// TypeError 4974: (38-63): Named argument "_data" does not match function declaration.
