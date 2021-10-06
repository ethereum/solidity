// SPDX-License-Identifier: GPL v3
type MyInt128 is int128;
type MyInt8 is int8;
contract C {
    // slot 0
    MyInt128 a;
    MyInt128 b;
    // slot 1
    MyInt128 c;
    MyInt8 d;
    MyInt8 e;
    MyInt8 f;
    MyInt8 g;
    // slot 2
    MyInt8 h;
}
