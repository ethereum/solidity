contract C {
    uint constant a = 2;
    uint constant b = a;
    function f() public pure {
        assembly {
            let x := b
        }
    }
}
// ----
// TypeError: (134-135): Only direct number constants are supported by inline assembly.
