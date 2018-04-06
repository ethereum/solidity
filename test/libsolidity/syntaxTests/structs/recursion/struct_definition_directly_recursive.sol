contract Test {
    struct MyStructName {
        address addr;
        MyStructName x;
    }
}
// ----
// TypeError: (20-93): Recursive struct definition.
