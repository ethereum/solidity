contract A {
    uint x;
    fallback() virtual external { x = 1; }
}
contract C is A {
    fallback() override external { x = 2; }
}
