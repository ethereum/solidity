// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
contract C {
    uint[2**255] a;
    uint[2**255] b;
}
// ----
// TypeError 7676: (60-114): Contract too large for storage.
// Warning 7325: (77-89): Type uint256[57896044618658097711785492504343953926634992332820282019728792003956564819968] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (97-109): Type uint256[57896044618658097711785492504343953926634992332820282019728792003956564819968] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
