pragma experimental "v0.5.0";
contract test {
    uint x = 1;
    function f() public {
        assembly {
            x := 2
        }
    }
}
// ----
// TypeError: (119-120): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
