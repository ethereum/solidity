{
    function allocate(size) -> p
    {
        let _1 := 0x40
        p := mload(_1)
        let _2 := add(p, size)
        let _3 := 0x40
        mstore(_3, _2)
    }
    function array_index_access(array, index) -> p_1
    {
        let _4 := 0x20
        let _5 := mul(index, _4)
        p_1 := add(array, _5)
    }
    let _6 := 0x20
    let _7 := allocate(_6)
    pop(_7)
    let _8 := 0x40
    let x := allocate(_8)
    let _9 := 2
    let _10 := 3
    let _11 := array_index_access(x, _10)
    mstore(_11, _9)
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     function allocate(size) -> p
//     {
//         let _1 := 0x40
//         p := mload(_1)
//         let _2 := add(p, size)
//         let _3 := _1
//         mstore(_1, _2)
//     }
//     function array_index_access(array, index) -> p_1
//     {
//         let _4 := 0x20
//         let _5 := mul(index, _4)
//         p_1 := add(array, _5)
//     }
//     let _6 := 0x20
//     let _7 := allocate(_6)
//     pop(_7)
//     let _8 := 0x40
//     let x := allocate(_8)
//     let _9 := 2
//     let _10 := 3
//     let _11 := array_index_access(x, _10)
//     mstore(_11, _9)
// }
