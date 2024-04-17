// SPDX-License-Identifier: GPL-3.0


// Regression for handling signedness, see issue #14792
contract C {
    mapping(int => int) v1;
    mapping(int => uint) v2;
    mapping(uint => int) v3;
    mapping(uint => uint) v4;
    mapping(bytes12 => int) v5;
    uint[5] a1;
    int[5] a2;

    function f() public {
        delete v1[0];
        delete v2[0];
        delete v3[0];
        delete v4[0];
        delete v5[0];
        delete a1[0];
        delete a2[0];
    }
}
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
