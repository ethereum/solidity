{
    mstore(0x40, memoryguard(128))
    sstore(0, g(sload(3)))
    function g(x) -> v {
      switch lt(x, 3)
      case 0 {
          v := f()
      }
      case 1 {
          v := g(sub(x,1))
      }
    }
    function f() -> v {
        let a1 := calldataload(mul(1,4))
        let a2 := calldataload(mul(2,4))
        let a3 := calldataload(mul(3,4))
        let a4 := calldataload(mul(4,4))
        let a5 := calldataload(mul(5,4))
        let a6 := calldataload(mul(6,4))
        let a7 := calldataload(mul(7,4))
        let a8 := calldataload(mul(8,4))
        let a9 := calldataload(mul(9,4))
        a1 := calldataload(mul(0,4))
        let a10 := calldataload(mul(10,4))
        let a11 := calldataload(mul(11,4))
        let a12 := calldataload(mul(12,4))
        let a13 := calldataload(mul(13,4))
        let a14 := calldataload(mul(14,4))
        let a15 := calldataload(mul(15,4))
        let a16 := calldataload(mul(16,4))
        let a17 := calldataload(mul(17,4))
        sstore(0, a1)
        sstore(mul(17,4), a17)
        sstore(mul(16,4), a16)
        sstore(mul(15,4), a15)
        sstore(mul(14,4), a14)
        sstore(mul(13,4), a13)
        sstore(mul(12,4), a12)
        sstore(mul(11,4), a11)
        sstore(mul(10,4), a10)
        sstore(mul(9,4), a9)
        sstore(mul(8,4), a8)
        sstore(mul(7,4), a7)
        sstore(mul(6,4), a6)
        sstore(mul(5,4), a5)
        sstore(mul(4,4), a4)
        sstore(mul(3,4), a3)
        sstore(mul(2,4), a2)
        sstore(mul(1,4), a1)
    }
}
// ----
// step: stackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xa0))
//     sstore(0, g(sload(3)))
//     function g(x) -> v
//     {
//         switch lt(x, 3)
//         case 0 { v := f() }
//         case 1 { v := g(sub(x, 1)) }
//     }
//     function f() -> v_1
//     {
//         mstore(0x80, calldataload(mul(1, 4)))
//         let a2 := calldataload(mul(2, 4))
//         let a3 := calldataload(mul(3, 4))
//         let a4 := calldataload(mul(4, 4))
//         let a5 := calldataload(mul(5, 4))
//         let a6 := calldataload(mul(6, 4))
//         let a7 := calldataload(mul(7, 4))
//         let a8 := calldataload(mul(8, 4))
//         let a9 := calldataload(mul(9, 4))
//         mstore(0x80, calldataload(mul(0, 4)))
//         let a10 := calldataload(mul(10, 4))
//         let a11 := calldataload(mul(11, 4))
//         let a12 := calldataload(mul(12, 4))
//         let a13 := calldataload(mul(13, 4))
//         let a14 := calldataload(mul(14, 4))
//         let a15 := calldataload(mul(15, 4))
//         let a16 := calldataload(mul(16, 4))
//         let a17 := calldataload(mul(17, 4))
//         sstore(0, mload(0x80))
//         sstore(mul(17, 4), a17)
//         sstore(mul(16, 4), a16)
//         sstore(mul(15, 4), a15)
//         sstore(mul(14, 4), a14)
//         sstore(mul(13, 4), a13)
//         sstore(mul(12, 4), a12)
//         sstore(mul(11, 4), a11)
//         sstore(mul(10, 4), a10)
//         sstore(mul(9, 4), a9)
//         sstore(mul(8, 4), a8)
//         sstore(mul(7, 4), a7)
//         sstore(mul(6, 4), a6)
//         sstore(mul(5, 4), a5)
//         sstore(mul(4, 4), a4)
//         sstore(mul(3, 4), a3)
//         sstore(mul(2, 4), a2)
//         sstore(mul(1, 4), mload(0x80))
//     }
// }
