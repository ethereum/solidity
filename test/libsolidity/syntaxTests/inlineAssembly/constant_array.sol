contract C {
    string constant x = "abc";
    function f() public pure {
        assembly {
            let a := x
        }
    }
}
// ----
// TypeError 7615: (115-116='x'): Only direct number constants and references to such constants are supported by inline assembly.
