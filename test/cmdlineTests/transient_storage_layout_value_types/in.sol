// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

type MyInt is int256;

enum MyEnum {
    None,
    Something,
    SomethingElse
}

contract C { }

contract A {
    uint transient x;
    int transient y;
    address transient addr;
    bool transient b;
    MyInt transient w;
    C transient c;
    MyEnum transient e;
    function() external transient p;
}
