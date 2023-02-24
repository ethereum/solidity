struct S {
    uint x;
}

contract C {
    uint a = 1000 S;
}
// ----
// TypeError 6469: (52-58): Type cannot be used as a literal suffix.
