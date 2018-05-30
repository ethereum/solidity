pragma experimental "v0.5.0";
contract c {
    uint8 x;
    function f() public {
        assembly { pop(x) }
    }
}
// ----
// TypeError: (105-106): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
