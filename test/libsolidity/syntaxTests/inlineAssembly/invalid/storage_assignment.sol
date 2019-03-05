contract test {
    uint x = 1;
    function f() public {
        assembly {
            x := 2
        }
    }
}
// ----
// TypeError: (89-90): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
