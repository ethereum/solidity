contract A {
    event X();
}
contract B is A {
    event X(uint);
}
// ----
