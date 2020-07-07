contract C {
    uint[2**64] x;
}
// ----
// Warning 3408: (17-30): Variable "x" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
