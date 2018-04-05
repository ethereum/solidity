contract A {
    uint constant c = B.c;
}
contract B {
    uint constant c = A.c;
}
// ----
// Some Error: This should fail with some error!