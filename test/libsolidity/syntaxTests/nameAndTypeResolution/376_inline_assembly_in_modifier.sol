pragma experimental "v0.5.0";
contract test {
    modifier m {
        uint a = 1;
        assembly {
            a := 2
        }
        _;
    }
    function f() public m {
    }
}
// ----
// Warning: (152-181): Function state mutability can be restricted to pure
