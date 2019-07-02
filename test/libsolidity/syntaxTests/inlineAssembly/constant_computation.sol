contract C {
    uint constant x = 2**20;
    function f() public pure {
        assembly {
            let a := x
        }
    }
}
// ----
