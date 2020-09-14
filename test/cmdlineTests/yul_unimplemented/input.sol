// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
    struct S {
        uint x;
    }
    S str;
    constructor() {
        delete str;
    }
}