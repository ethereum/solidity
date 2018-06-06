contract C {
    struct S { uint[2**30] x; uint[2**50] y; }
    S[2**20] x;
}
// ----
// Warning: (64-74): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
