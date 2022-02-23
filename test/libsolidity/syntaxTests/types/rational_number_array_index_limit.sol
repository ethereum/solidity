contract c {
    uint[2**253] data;
}
// ----
// Warning 7325: (17-29): Type uint256[14474011154664524427946373126085988481658748083205070504932198000989141204992] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
