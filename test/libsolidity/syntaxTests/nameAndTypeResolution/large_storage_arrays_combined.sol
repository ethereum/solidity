contract C {
    uint[200][200][2**30][][2**30] x;
}
// ----
// Warning: (17-49): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
