{
    function f() -> x {
        x := mload(0)
    }

    // In pure Yul optimization in presence of msize is allowed.
    // Everything in this file should get optimized out.
    pop(msize())

    let x := 0
    let y := x
    mstore(0, f())
}
