// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

type MyInt is int128;

enum MyEnum {
    None,
    Something,
    SomethingElse
}

contract C { }

contract A {
    uint64 transient x;
    uint128 transient y;
    uint128 transient z;
    address transient addr;
    bool transient b;
    MyInt transient w;
    C transient c;
    MyEnum transient e;
    function() external transient p;
}
