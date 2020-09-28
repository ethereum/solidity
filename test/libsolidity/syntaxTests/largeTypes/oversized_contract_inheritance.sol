// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
contract C {
    uint[2**255] a;
}
contract D is C {
    uint[2**255] b;
}
// ----
// TypeError 7676: (95-134): Contract requires too much storage.
