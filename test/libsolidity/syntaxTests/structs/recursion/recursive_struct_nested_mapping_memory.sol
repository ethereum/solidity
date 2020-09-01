library a {
    struct b {
        mapping (uint => b) c ;
    }
    // Segfaults in https://github.com/ethereum/solidity/issues/9443
    function d(b memory) public {}
}
// ----
// TypeError 4103: (149-157): Recursive structs can only be passed as storage pointers to libraries, not as memory objects to contract functions.
// TypeError 4061: (149-157): Type struct a.b is only valid in storage because it contains a (nested) mapping.
