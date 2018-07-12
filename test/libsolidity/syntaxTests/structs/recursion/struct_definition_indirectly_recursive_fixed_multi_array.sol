contract Test {
    struct MyStructName1 {
        address addr;
        uint256 count;
        MyStructName2[1][1] x;
    }
    struct MyStructName2 {
        MyStructName1 x;
    }
}
// ----
// TypeError: (20-124): Recursive struct definition.
