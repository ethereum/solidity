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
// TypeError: (80-81): Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.
