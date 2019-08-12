contract C {
    string constant x = "abc";
    function f() public pure {
        assembly {
            let a := x
        }
    }
}
// ----
// TypeError: (115-116): Only direct number constants are supported by inline assembly.
