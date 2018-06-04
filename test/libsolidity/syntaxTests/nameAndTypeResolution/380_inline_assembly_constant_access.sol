pragma experimental "v0.5.0";
contract test {
    uint constant x = 1;
    function f() public {
        assembly {
            let y := x
        }
    }
}
// ----
// TypeError: (137-138): Constant variables not supported by inline assembly.
