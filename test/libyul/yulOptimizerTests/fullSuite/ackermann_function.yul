// https://en.wikipedia.org/wiki/Ackermann_function
// Test to see how FunctionSpecializer deals with functions that are too recursive / resource intensive.
{
    // 5
    sstore(0, A(2, 1))
    // 7
    sstore(1, A(2, 2))

    // Too many unrolling needed. In arbitrary precision, the value is 2**65536 - 3.
    sstore(2, A(4, 2))

    function A(m, n) -> ret {
        switch m
        case 0 { ret := add(n, 1) }
        default {
            switch n
            case 0 { ret := A(sub(m, 1), 1) }
            default { ret := A(sub(m, 1), A(m, sub(n, 1))) }
        }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, 5)
//         sstore(1, add(2, 5))
//         let _1 := not(0)
//         let _2 := A_985(A_4878(A_4886(add(5, _1))))
//         let ret := 0
//         switch _2
//         case 0 { ret := 5 }
//         default {
//             let _3 := A_985(add(_2, _1))
//             let ret_1 := 0
//             switch _3
//             case 0 { ret_1 := 3 }
//             default {
//                 ret_1 := A_2511(A_2505(add(_3, _1)))
//             }
//             ret := ret_1
//         }
//         sstore(2, ret)
//     }
//     function A_20862() -> ret
//     {
//         switch 1
//         case 0 { ret := add(ret, 1) }
//         default {
//             let _1 := add(1, not(0))
//             let ret_1 := 0
//             switch _1
//             case 0 { ret_1 := 2 }
//             default {
//                 ret_1 := A(add(1, not(1)), A_294(_1))
//             }
//             ret := ret_1
//         }
//     }
//     function A_20864() -> ret
//     {
//         switch 2
//         case 0 { ret := add(ret, 1) }
//         default {
//             let _1 := add(2, not(0))
//             let ret_1 := 0
//             switch _1
//             case 0 { ret_1 := 2 }
//             default {
//                 ret_1 := A(add(2, not(1)), A_294(_1))
//             }
//             ret := ret_1
//         }
//     }
//     function A_20874() -> ret
//     {
//         switch 1
//         case 0 { ret := add(ret, 1) }
//         default {
//             let _1 := add(1, not(0))
//             let ret_1 := 0
//             switch _1
//             case 0 { ret_1 := 2 }
//             default {
//                 ret_1 := A(add(1, not(1)), A_294(_1))
//             }
//             ret := ret_1
//         }
//     }
//     function A_294(m) -> ret
//     {
//         switch m
//         case 0 { ret := add(ret, 1) }
//         default {
//             let _1 := add(m, not(0))
//             let ret_1 := 0
//             switch _1
//             case 0 { ret_1 := 2 }
//             default {
//                 ret_1 := A(add(m, not(1)), A_294(_1))
//             }
//             ret := ret_1
//         }
//     }
//     function A_985(n) -> ret
//     {
//         switch n
//         case 0 { ret := 5 }
//         default {
//             let ret_1 := 0
//             switch add(n, not(0))
//             case 0 { ret_1 := 5 }
//             default {
//                 let ret_2 := 0
//                 switch add(n, not(1))
//                 case 0 { ret_2 := add(2, 3) }
//                 default {
//                     let ret_3 := 0
//                     switch add(n, not(2))
//                     case 0 {
//                         ret_3 := add(A_13949(A_13949(A_20856())), 1)
//                     }
//                     default {
//                         let ret_4 := 0
//                         switch add(n, not(3))
//                         case 0 { ret_4 := add(A_20857(), 1) }
//                         default {
//                             let ret_5 := 0
//                             switch add(n, not(4))
//                             case 0 { ret_5 := A_20858() }
//                             default {
//                                 ret_5 := A_13953(A_13952(add(n, not(5))))
//                             }
//                             ret_4 := A_11648(ret_5)
//                         }
//                         ret_3 := A_9730(ret_4)
//                     }
//                     ret_2 := A_7274(ret_3)
//                 }
//                 ret_1 := A_4886(ret_2)
//             }
//             ret := A_2505(ret_1)
//         }
//     }
//     function A_2505(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_4878(A_4886(add(n, not(0))))
//         }
//     }
//     function A_2511(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_4878(add(n, not(0))), 1)
//         }
//     }
//     function A_4878(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_7272(add(n, not(0))), 1)
//         }
//     }
//     function A_4886(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_7272(A_7274(add(n, not(0))))
//         }
//     }
//     function A_7272(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_9725(add(n, not(0))), 1)
//         }
//     }
//     function A_7274(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_9725(A_9730(add(n, not(0))))
//         }
//     }
//     function A_9725(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_11646(add(n, not(0))), 1)
//         }
//     }
//     function A_9730(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_11646(A_11648(add(n, not(0))))
//         }
//     }
//     function A_11646(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_13951(add(n, not(0))), 1)
//         }
//     }
//     function A_11648(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_13951(A_13953(add(n, not(0))))
//         }
//     }
//     function A_13949(n) -> ret
//     { ret := add(n, 1) }
//     function A_20857() -> ret
//     {
//         switch 2
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_16299(add(2, not(0))), 1)
//         }
//     }
//     function A_20858() -> ret
//     {
//         switch 3
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_16299(add(3, not(0))), 1)
//         }
//     }
//     function A_13951(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_16299(add(n, not(0))), 1)
//         }
//     }
//     function A_13952(n) -> ret
//     {
//         switch n
//         case 0 { ret := A_20861() }
//         default {
//             let ret_1 := 0
//             switch add(n, not(0))
//             case 0 {
//                 ret_1 := A_18381(A_20863(A_20862()))
//             }
//             default {
//                 let ret_2 := 0
//                 switch add(n, not(1))
//                 case 0 { ret_2 := A_20865(A_20864()) }
//                 default {
//                     ret_2 := A_20867(A_20866(add(n, not(2))))
//                 }
//                 ret_1 := A_18383(ret_2)
//             }
//             ret := A_16304(ret_1)
//         }
//     }
//     function A_13953(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_16299(A_16304(add(n, not(0))))
//         }
//     }
//     function A_20856() -> ret
//     {
//         switch ret
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_18381(add(0, not(0))), 1)
//         }
//     }
//     function A_20861() -> ret
//     {
//         switch 3
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_18381(add(3, not(0))), 1)
//         }
//     }
//     function A_16299(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := add(A_18381(add(n, not(0))), 1)
//         }
//     }
//     function A_16304(n) -> ret
//     {
//         switch n
//         case 0 { ret := 3 }
//         default {
//             ret := A_18381(A_18383(add(n, not(0))))
//         }
//     }
//     function A_18381(n) -> ret
//     {
//         switch n
//         case 0 { ret := 2 }
//         default {
//             ret := A_20869(A_20868(add(n, not(0))))
//         }
//     }
//     function A_18383(n) -> ret
//     {
//         switch n
//         case 0 { ret := A_20875(A_20874()) }
//         default {
//             ret := A_20877(A_20876(add(n, not(0))))
//         }
//     }
//     function A_20863(n) -> ret
//     {
//         switch ret
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(0, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(0, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(0, _2), A(0, add(n, _2)))
//             }
//         }
//     }
//     function A_20865(n) -> ret
//     {
//         let m := 1
//         switch m
//         case 0 { ret := add(n, m) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
//     function A_20866(n) -> ret
//     {
//         switch 3
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(3, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(3, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(3, _2), A(3, add(n, _2)))
//             }
//         }
//     }
//     function A_20867(n) -> ret
//     {
//         let m := 2
//         switch m
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := m }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
//     function A_20868(n) -> ret
//     {
//         let m := 1
//         switch m
//         case 0 { ret := add(n, m) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
//     function A_20869(n) -> ret
//     {
//         switch ret
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(0, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(0, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(0, _2), A(0, add(n, _2)))
//             }
//         }
//     }
//     function A_20875(n) -> ret
//     {
//         switch ret
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(0, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(0, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(0, _2), A(0, add(n, _2)))
//             }
//         }
//     }
//     function A_20876(n) -> ret
//     {
//         let m := 2
//         switch m
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := m }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
//     function A_20877(n) -> ret
//     {
//         let m := 1
//         switch m
//         case 0 { ret := add(n, m) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
//     function A(m, n) -> ret
//     {
//         switch m
//         case 0 { ret := add(n, 1) }
//         default {
//             switch n
//             case 0 {
//                 let _1 := add(m, not(0))
//                 let ret_1 := 0
//                 switch _1
//                 case 0 { ret_1 := 2 }
//                 default {
//                     ret_1 := A(add(m, not(1)), A_294(_1))
//                 }
//                 ret := ret_1
//             }
//             default {
//                 let _2 := not(0)
//                 ret := A(add(m, _2), A(m, add(n, _2)))
//             }
//         }
//     }
// }
