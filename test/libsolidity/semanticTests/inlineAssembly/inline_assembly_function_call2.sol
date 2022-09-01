contract C {
    function f() public {
        assembly {
            let d := 0x10

            function asmfun(a, b, c) -> x, y, z {
                x := a
                y := b
                z := 7
            }
            let a1, b1, c1 := asmfun(1, 2, 3)
            mstore(0x00, a1)
            mstore(0x20, b1)
            mstore(0x40, c1)
            mstore(0x60, d)
            return (0, 0x80)
        }
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 0x1, 0x2, 0x7, 0x10
