// SPDX-License-Identifier: GPL-3.0
pragma solidity > 0.0;

contract D { }

contract C {
    uint transient x;
    uint y;
    address transient w;
    int z;
    bool transient b;
    D transient d;
}