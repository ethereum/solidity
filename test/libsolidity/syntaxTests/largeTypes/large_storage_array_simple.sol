contract C {
    uint[2**64] x;
}
// ----
// Warning 7325: (17-28): Type uint256[18446744073709551616] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
