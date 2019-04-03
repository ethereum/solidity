{
    let x := f(0)
    function f(y) -> r {
        r := mload(mul(6, add(y, 0x20)))
    }
    for { let a := 2 } lt(a, mload(a)) { a := add(a, mul(a, 2)) } {
        let b := mul(add(a, f(a)), 4)
        sstore(b, mul(b, 2))
    }
}
// ====
// step: splitJoin
// ----
// {
//     let x := f(0)
//     function f(y) -> r
//     {
//         r := mload(mul(6, add(y, 0x20)))
//     }
//     for {
//         let a := 2
//     }
//     lt(a, mload(a))
//     {
//         a := add(a, mul(a, 2))
//     }
//     {
//         let b := mul(add(a, f(a)), 4)
//         sstore(b, mul(b, 2))
//     }
// }
