{
    let _1 := 0
    let _33 := calldataload(_1)
    let sum_50_141 := _1
    let sum_50_146 := _1
    let sum_50 := _1
    let length_51 := calldataload(_33)
    let i_53_142 := _1
    let i_53_147 := _1
    let i_53 := _1
    for { }
    1
    {
        let _108 := 1
        let i_53_121 := add(i_53, _108)
        let i_53_144 := i_53_121
        let i_53_149 := i_53_121
        i_53 := i_53_121
    }
    {
        let _109 := lt(i_53, length_51)
        let _110 := iszero(_109)
        if _110 { break }
        let _114_128 := iszero(_109)
        if _114_128 { revert(_1, _1) }
        let _13_129 := 0x20
        let _115_130 := mul(i_53, _13_129)
        let _116_131 := add(_33, _115_130)
        let _117_132 := add(_116_131, _13_129)
        let v_122_133 := calldataload(_117_132)
        let sum_50_120 := add(sum_50, v_122_133)
        let sum_50_143 := sum_50_120
        let sum_50_148 := sum_50_120
        sum_50 := sum_50_120
    }
    sstore(_1, sum_50)
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let _1 := 0
//     let _33 := calldataload(_1)
//     let sum_50_141 := _1
//     let sum_50_146 := _1
//     let sum_50 := _1
//     let length_51 := calldataload(_33)
//     let i_53_142 := _1
//     let i_53_147 := _1
//     let i_53 := _1
//     for { }
//     1
//     {
//         let _108 := 1
//         let i_53_121 := add(i_53, _108)
//         let i_53_144 := i_53_121
//         let i_53_149 := i_53_121
//         i_53 := i_53_121
//     }
//     {
//         let _109 := lt(i_53, length_51)
//         let _110 := iszero(_109)
//         if _110 { break }
//         let _114_128 := _110
//         if _110 { revert(_1, _1) }
//         let _13_129 := 0x20
//         let _115_130 := mul(i_53, _13_129)
//         let _116_131 := add(_33, _115_130)
//         let _117_132 := add(_116_131, _13_129)
//         let v_122_133 := calldataload(_117_132)
//         let sum_50_120 := add(sum_50, v_122_133)
//         let sum_50_143 := sum_50_120
//         let sum_50_148 := sum_50_120
//         sum_50 := sum_50_120
//     }
//     sstore(_1, sum_50)
// }
