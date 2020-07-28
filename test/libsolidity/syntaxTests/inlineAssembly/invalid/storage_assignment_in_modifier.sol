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
// TypeError 1408: (80-81): Only local variables are supported. To access storage variables, use the .slot and .offset suffixes.
