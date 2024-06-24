// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
type MyInt is int8;
contract A {
    uint public transient x;
    uint public y;
    int private transient z;
    int private w;
    address transient addr;
    address d;
    bool transient b;
    bool c;
    MyInt i;
    MyInt transient j;
}
