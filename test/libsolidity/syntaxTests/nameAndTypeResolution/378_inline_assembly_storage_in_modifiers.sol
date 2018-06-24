pragma experimental "v0.5.0";
contract test {
    uint x = 1;
    modifier m {
        assembly {
            x := 2
        }
        _;
    }
    function f() public m {
    }
}
// ----
// TypeError: (110-111): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
