{
    main(0, 0)

    function main(a, b) {
        for {} 1 {}
        {
            if iszero(a) { break }

            let mid := avg(a, a)
            switch a
            case 0 {
                a := mid
            }
            default {
                sstore(0, mid)
            }
        }
    }

    function avg(x, y) -> var {
        // NOTE: Variable names should not affect CSE.
        // This should not be optimized differently than name_dependent_cse_bug_part_2_pre_shanghai.yul.
        // `let mid := var` should be present in both or missing in both.
        let _1 := add(x, y)
        var := add(_1, _1)
    }
}
// ====
// EVMVersion: <shanghai
// ----
// step: fullSuite
//
// {
//     {
//         let a := 0
//         let a_1 := 0
//         for { } a_1 { }
//         {
//             let _1 := add(a_1, a_1)
//             let var := add(_1, _1)
//             switch a_1
//             case 0 { a_1 := var }
//             default { sstore(a, var) }
//         }
//     }
// }
