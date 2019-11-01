{
    sstore(0, array_sum(calldataload(0)))

    function array_sum(x) -> sum {
        let length := calldataload(x)
        for { let i := 0 } lt(i, length) { i := add(i, 1) } {
            sum := add(sum, array_load(x, i))
        }
    }
    function array_load(x, i) -> v {
        let len := calldataload(x)
        if iszero(lt(i, len)) { revert(0, 0) }
        let data := add(x, 0x20)
        v := calldataload(add(data, mul(i, 0x20)))
    }
}
// ====
// step: fullSuite
// ----
// {
//     {
//         let _1 := calldataload(0)
//         let sum := 0
//         let i := sum
//         for { } lt(i, calldataload(_1)) { i := add(i, 1) }
//         {
//             sum := add(sum, calldataload(add(add(_1, mul(i, 0x20)), 0x20)))
//         }
//         sstore(0, sum)
//     }
// }
