{
    // references parameter 1 times in function body
    function ref1(a) -> x { x := add(a, 1) }
    // references parameter 3 times in function body
    function ref3(a) -> x { x := add(a, mul(a, a)) }
    let y1 := ref1(calldatasize())
    let y2 := ref3(calldatasize())
    let y3 := ref1(0xff)
    let y4 := ref3(0xff)
    let y5 := ref1(0x123)
    let y6 := ref3(0x123)
    let y7 := ref1(mload(42))
    let y8 := ref3(mload(42))
    let y9 := ref1(ref3(7))
    let y10:= ref3(ref1(7))
    let y11:= ref1(y1)
    let y12:= ref3(y1)
}
// ====
// step: expressionInliner
// ----
// {
//     function ref1(a) -> x
//     { x := add(a, 1) }
//     function ref3(a_1) -> x_2
//     {
//         x_2 := add(a_1, mul(a_1, a_1))
//     }
//     let y1 := add(calldatasize(), 1)
//     let y2 := add(calldatasize(), mul(calldatasize(), calldatasize()))
//     let y3 := add(0xff, 1)
//     let y4 := add(0xff, mul(0xff, 0xff))
//     let y5 := add(0x123, 1)
//     let y6 := ref3(0x123)
//     let y7 := ref1(mload(42))
//     let y8 := ref3(mload(42))
//     let y9 := add(add(7, mul(7, 7)), 1)
//     let y10 := ref3(add(7, 1))
//     let y11 := add(y1, 1)
//     let y12 := add(y1, mul(y1, y1))
// }
