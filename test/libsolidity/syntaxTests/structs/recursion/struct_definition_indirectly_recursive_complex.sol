contract Test {
    struct MyStructName1 {
        address addr;
        uint256 count;
        MyStructName4[1] x;
    }
    struct MyStructName2 {
        MyStructName1 x;
    }
    struct MyStructName3 {
        MyStructName2[1] x;
    }
    struct MyStructName4 {
        MyStructName3 x;
    }
}
// ----
// TypeError: (20-121): Recursive struct definition.
