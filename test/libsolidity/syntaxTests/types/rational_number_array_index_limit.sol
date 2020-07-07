contract c {
    uint[2**253] data;
}
// ----
// Warning 3408: (17-34): Variable "data" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
