{
    let x := calldataload(0)
    let y := calldataload(1)
    let a := keccak256(x, y)
    if calldataload(2) {
        a := 8
    }
    let b := keccak256(x, y)
    sstore(b, 2)
    if calldataload(3) {
        x := 8
    }
    let c := keccak256(x, y)
    sstore(c, 2)
    if calldataload(4) {
        y := 8
    }
    let d := keccak256(x, y)
    sstore(d, 2)
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(0)
//         let y := calldataload(1)
//         let a := keccak256(x, y)
//         let _3 := 2
//         if calldataload(_3) { a := 8 }
//         sstore(keccak256(x, y), _3)
//         if calldataload(3) { x := 8 }
//         sstore(keccak256(x, y), _3)
//         if calldataload(4) { y := 8 }
//         sstore(keccak256(x, y), _3)
//     }
// }
