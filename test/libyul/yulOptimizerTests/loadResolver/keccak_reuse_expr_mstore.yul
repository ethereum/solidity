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
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let a := calldataload(0)
//         sstore(f(keccak256(0, a)), keccak256(0, a))
//         sstore(f(keccak256(0, a)), keccak256(0, a))
//         sstore(keccak256(0, a), f(keccak256(0, a)))
//     }
//     function f(x) -> y
//     {
//         mstore(x, 2)
//         y := mload(8)
//     }
// }
