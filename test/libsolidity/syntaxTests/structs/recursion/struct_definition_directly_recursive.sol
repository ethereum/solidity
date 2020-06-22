contract Test {
    struct MyStructName {
        address addr;
        MyStructName x;
    }
}
// ----
// TypeError 2046: (20-93): Recursive struct definition.
