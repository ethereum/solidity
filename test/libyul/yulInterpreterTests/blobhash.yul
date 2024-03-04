{
    sstore(0, blobhash(0))
    sstore(1, blobhash(1))
    sstore(2, blobhash(2)) // should store 0 since EVMHost has only two blob hashes injected in the block the transaction is being executed.
}
// ====
// EVMVersion: >=cancun
// ----
// Trace:
// Memory dump:
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: 014916dd28fc4c10d78e287ca5d9cc51ee1ae73cbfde08c6b37324cbfaac8bc5
//   0000000000000000000000000000000000000000000000000000000000000001: 0167d3dbed802941483f1afa2a6bc68de5f653128aca9bf1461c5d0a3ad36ed2
// Transient storage dump:
