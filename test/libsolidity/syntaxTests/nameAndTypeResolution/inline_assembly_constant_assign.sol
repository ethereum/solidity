pragma experimental "v0.5.0";
contract test {
    uint constant x = 1;
    function f() public {
        assembly {
            x := 2
        }
    }
}
// ----
// TypeError: (128-129): Constant variables not supported by inline assembly.
