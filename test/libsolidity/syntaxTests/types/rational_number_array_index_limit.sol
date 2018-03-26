contract c {
    uint[2**253] data;
}
// ----
// Warning: (17-34): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
