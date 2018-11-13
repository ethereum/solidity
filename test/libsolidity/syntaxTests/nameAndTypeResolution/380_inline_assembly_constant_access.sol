contract test {
    uint constant x = 1;
    function f() public {
        assembly {
            let y := x
        }
    }
}
// ----
// TypeError: (107-108): Constant variables not supported by inline assembly.
