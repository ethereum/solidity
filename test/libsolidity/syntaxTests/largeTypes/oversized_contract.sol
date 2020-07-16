// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
contract C {
    uint[2**255] a;
    uint[2**255] b;
}
// ----
// TypeError 7676: (60-114): Contract too large for storage.
// Warning 3408: (77-91): Variable "a" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (97-111): Variable "b" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
