// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
contract C {
    uint[2**255] a;
    uint[2**255] b;
}
// ----
// TypeError 7676: (60-114): Contract requires too much storage.
