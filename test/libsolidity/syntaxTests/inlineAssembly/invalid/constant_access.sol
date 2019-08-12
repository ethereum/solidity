contract test {
    uint constant x = 1;
    function f() public pure {
        assembly {
            let y := x
        }
    }
}
// ----
