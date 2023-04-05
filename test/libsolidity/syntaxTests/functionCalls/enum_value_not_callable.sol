enum E { A, B, C }

contract C {
    uint a = E.B(1000);
}
// ----
// TypeError 5704: (46-55): Enum is not callable.
