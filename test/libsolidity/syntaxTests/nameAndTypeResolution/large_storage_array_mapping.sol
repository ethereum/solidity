contract C {
    mapping(uint => uint[2**100]) x;
}
// ----
// Warning: (17-48): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
