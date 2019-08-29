{
    sstore(0, 123213)
    for {let x := 0 let y} lt(x, sload(0)) {
        x := add(x, 1)} {y := add(x, y)
    }
}
// ====
// step: loadResolver
// ----
// {
//     let _1 := 123213
//     let _2 := 0
//     sstore(_2, _1)
//     let x := _2
//     let y
//     for { } lt(x, _1) { x := add(x, 1) }
//     { y := add(x, y) }
// }
