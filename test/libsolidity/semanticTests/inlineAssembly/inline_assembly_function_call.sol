contract C {
    function f() public {
        assembly {
            function asmfun(a, b, c) -> x, y, z {
                x := a
                y := b
                z := 7
            }
            let a1, b1, c1 := asmfun(1, 2, 3)
            mstore(0x00, a1)
            mstore(0x20, b1)
            mstore(0x40, c1)
            return (0, 0x60)
        }
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1, 2, 7
