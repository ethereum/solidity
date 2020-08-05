contract test {
    uint x = 1;
    function f() public {
        assembly {
            x := 2
        }
    }
}
// ----
// TypeError 1408: (89-90): Only local variables are supported. To access storage variables, use the .slot and .offset suffixes.
