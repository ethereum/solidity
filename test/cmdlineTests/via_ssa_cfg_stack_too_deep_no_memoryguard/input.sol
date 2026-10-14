// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    uint[17] s;

    function f(
        uint a0, uint a1, uint a2, uint a3, uint a4, uint a5, uint a6, uint a7, uint a8,
        uint a9, uint a10, uint a11, uint a12, uint a13, uint a14, uint a15, uint a16
    ) internal {
        s[0] = a0; s[1] = a1; s[2] = a2; s[3] = a3; s[4] = a4; s[5] = a5; s[6] = a6; s[7] = a7; s[8] = a8;
        s[9] = a9; s[10] = a10; s[11] = a11; s[12] = a12; s[13] = a13; s[14] = a14; s[15] = a15; s[16] = a16;
    }

    function g() external {
        f(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16]);
        assembly { mstore(0, 0) }
    }
}
