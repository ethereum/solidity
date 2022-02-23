{
    let x := calldataload(0)
    for { } lt(x, 10) { x := add(x, 1) } {
        if eq(x, 2) { break }
        if eq(x, 4) { continue }
    }
    sstore(0, x)
}
