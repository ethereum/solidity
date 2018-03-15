contract Test {
    struct MyStructName {
        address addr;
        MyStructName x;
    }
}
// ----
// TypeError: Recursive struct definition.
