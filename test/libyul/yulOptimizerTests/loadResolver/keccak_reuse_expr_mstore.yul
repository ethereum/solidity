{
    let a := calldataload(0)
    sstore(f(keccak256(0, a)), keccak256(0, a))
    sstore(f(keccak256(0, a)), keccak256(0, a))
    sstore(keccak256(0, a), f(keccak256(0, a)))

    function f(x) -> y {
        mstore(x, 2)
        y := mload(8)
    }
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let a := calldataload(_1)
//         let _3 := keccak256(_1, a)
//         sstore(f(_3), _3)
//         let _8 := keccak256(_1, a)
//         sstore(f(_8), _8)
//         sstore(keccak256(_1, a), f(keccak256(_1, a)))
//     }
//     function f(x) -> y
//     {
//         mstore(x, 2)
//         y := mload(8)
//     }
// }
