contract c {
    uint8 x;
    function f() public {
        assembly { pop(x) }
    }
}
// ----
// TypeError 1408: (75-76): Only local variables are supported. To access storage variables, use the .slot and .offset suffixes.
