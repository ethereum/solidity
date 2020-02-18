contract C {
    enum E { A, B, C }
    mapping(C => bool) a;
    mapping(address => bool) b;
    mapping(E => bool) c;
}

// ----
