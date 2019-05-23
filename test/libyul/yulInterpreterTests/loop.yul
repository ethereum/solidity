{
    for { let x := 2 } lt(x, 10) { x := add(x, 1) } {
        mstore(mul(x, 5), mul(x, 0x1000))
    }
}
// ----
// Trace:
// Memory dump:
//     40: 0000000000000000000000900000000000000000000000000000000000000000
// Storage dump:
