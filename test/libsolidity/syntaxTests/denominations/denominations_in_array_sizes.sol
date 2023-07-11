contract C {
    uint[42 wei] a;
    uint[42 gwei] b;
    uint[42 ether] c;
    uint[42 seconds] d;
    uint[42 minutes] e;
    uint[42 hours] f;
    uint[42 days] g;
    uint[42 weeks] h;
}
// ----
// Warning 7325: (58-72): Type uint256[42000000000000000000] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
