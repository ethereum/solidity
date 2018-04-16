contract Test {
    struct MyStructName1 {
        address addr;
        uint256 count;
        mapping(uint => MyStructName1) x;
    }
}
