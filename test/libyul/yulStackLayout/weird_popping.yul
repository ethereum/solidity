{
    let x_1 := calldataload(0)
    let x_2 := calldataload(1)
    let x_3 := calldataload(2)
    let x_4 := calldataload(3)
    let x_5 := calldataload(4)
    if lt(x_1, x_2) {
        let z := 42
        if lt(calldataload(5), calldataload(6)) {
            z := mul(z, 2)
        }
        revert(z, z)
    }
    sstore(add(x_1, add(x_2, add(x_3, add(x_4, add(x_5, 0))))), 42)
}
// ----
