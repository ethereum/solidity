contract c {
    uint8 x;
    function f() public {
        assembly { pop(x) }
    }
}
// ----
// TypeError: (75-76): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
