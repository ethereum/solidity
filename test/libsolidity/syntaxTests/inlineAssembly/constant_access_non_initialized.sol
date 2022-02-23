contract C {
    uint constant x;
    function f() public pure {
        assembly {
            let c1 := x
        }
    }
}
// ----
// TypeError 4266: (17-32): Uninitialized "constant" variable.
// TypeError 3224: (106-107): Constant has no value.
