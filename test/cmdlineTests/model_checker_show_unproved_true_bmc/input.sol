// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract C {
    struct S {
        uint x;
    }
    S s;
    function f(bool b) public {
        s.x |= b ? 1 : 2;
        assert(s.x > 0);
    }
}
