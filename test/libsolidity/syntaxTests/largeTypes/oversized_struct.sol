// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
contract C {
    struct S {
        uint[2**255] a;
        uint[2**255] b;
    }
    S s;
}
// ----
// TypeError 7676: (60-152): Contract requires too much storage.
// TypeError 1534: (146-149): Type too large for storage.
