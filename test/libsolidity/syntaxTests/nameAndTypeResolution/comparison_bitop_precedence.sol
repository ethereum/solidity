contract First {
    function fun() public returns (bool ret) {
        return 1 & 2 == 8 & 9 && 1 ^ 2 < 4 | 6;
    }
}
// ----
// Warning: (21-117): Function state mutability can be restricted to pure
