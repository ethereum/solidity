{
    sstore(0, 123213)
    for {let x := 0 let y} lt(x, sload(0)) {
        x := add(x, 1)} {y := add(x, y)
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         sstore(0, 123213)
//         let x := 0
//         let y
//         for { } lt(x, sload(0)) { x := add(x, 1) }
//         { y := add(x, y) }
//     }
// }
