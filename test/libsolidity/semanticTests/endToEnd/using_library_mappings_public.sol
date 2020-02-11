library Lib {
    function set(mapping(uint => uint) storage m, uint key, uint value) public {
        m[key] = value;
    }
}
contract Test {
    mapping(uint => uint) m1;
    mapping(uint => uint) m2;

    function f() public returns(uint, uint, uint, uint, uint, uint) {
        Lib.set(m1, 0, 1);
        Lib.set(m1, 2, 42);
        Lib.set(m2, 0, 23);
        Lib.set(m2, 2, 99);
        return (m1[0], m1[1], m1[2], m2[0], m2[1], m2[2]);
    }
}

// ----

library Lib {
    function set(mapping(uint => uint) storage m, uint key, uint value) public {
        m[key] = value;
    }
}
contract Test {
    mapping(uint => uint) m1;
    mapping(uint => uint) m2;

    function f() public returns(uint, uint, uint, uint, uint, uint) {
        Lib.set(m1, 0, 1);
        Lib.set(m1, 2, 42);
        Lib.set(m2, 0, 23);
        Lib.set(m2, 2, 99);
        return (m1[0], m1[1], m1[2], m2[0], m2[1], m2[2]);
    }
}

// ----
// f() -> 1, 0, 42, 23, 0, 99
// f():"" -> "1, 0, 42, 23, 0, 99"
