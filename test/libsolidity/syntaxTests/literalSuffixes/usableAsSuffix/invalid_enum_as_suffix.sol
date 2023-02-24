enum E { A, B, C}

contract C {
    uint a = 1000 E;
}
// ----
// TypeError 6469: (45-51): Type cannot be used as a literal suffix.
