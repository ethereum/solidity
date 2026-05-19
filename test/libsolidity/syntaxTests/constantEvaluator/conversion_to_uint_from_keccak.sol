bytes32 constant CONST_HASH = keccak256("1234abcd");

contract A layout at uint(keccak256("example")) {}
contract C {
    uint[uint(CONST_HASH)] array;
}
// ----
// Warning 7325: (122-144): Type uint256[22668996584266725980521143636517830746133512938818882126204489763938146811214] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
