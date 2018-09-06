contract Test {
    struct MyStructName {
        address addr;
        MyStructName[1] x;
    }
}
// ----
// TypeError: (20-96): Recursive struct definition.
