contract Test {
    struct MyStructName1 {
        address addr;
        uint256 count;
        MyStructName2 x;
    }
    struct MyStructName2 {
        MyStructName1[] x;
    }
}
// ----
