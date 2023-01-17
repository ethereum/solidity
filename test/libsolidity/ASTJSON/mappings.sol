contract C {
    enum E { A, B, C }
    mapping(C => bool) a;
    mapping(address => bool) b;
    mapping(E => bool) c;
    mapping(address keyAddress => uint256 value) d;
}

// ----
