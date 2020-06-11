contract C {
    struct P0 { uint256[2**63] x; }
    struct S0 {
        P0[2**62] y;
        P0 x;
    }
    S0 s0;

    struct P1 { uint256[2**63] x; }
    struct S1 {
        P1 x;
        P1[2**62] y;
    }
    S1 s1;
}
// ----
// Warning 3408: (110-115): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (215-220): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
