contract C {
    uint constant x;
    function f() public pure {
        assembly {
            let c1 := x
        }
    }
}
// ----
// TypeError 4266: (17-32='uint constant x'): Uninitialized "constant" variable.
// TypeError 3224: (106-107='x'): Constant has no value.
