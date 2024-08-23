{
    function foo(x, y) -> r {
        switch mod(add(x, y), 2)
        case 0 {
            r := 1
        }
        case 1
        {
            sstore(0, 1)
            r := 2
        }
    }

    function a() -> k {
        k := foo(1, 1)
    }

    function b() -> h {
        h := foo(2, 1)
    }
}

// ----
// step: constantFunctionEvaluator
//
// {
//     function foo(x, y) -> r
//     {
//         switch mod(add(x, y), 2)
//         case 0 { r := 1 }
//         case 1 {
//             sstore(0, 1)
//             r := 2
//         }
//     }
//     function a() -> k
//     { k := 1 }
//     function b() -> h
//     { h := foo(2, 1) }
// }
