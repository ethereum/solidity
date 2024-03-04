{
    function f() -> x {
        x := mload(0)
    }

    // In pure Yul without optimizer presence of msize disables stack optimization.
    // This file should remain untouched when passed through the optimizer.
    pop(msize())

    let x := 0
    let y := x
    mstore(0, f())
}
