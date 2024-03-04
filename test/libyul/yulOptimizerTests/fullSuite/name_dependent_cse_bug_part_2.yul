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
        // This should not be optimized differently than name_dependent_cse_bug_part_1.yul.
        // `let mid := var` should be present in both or missing in both.
        let _2 := add(x, y)
        var := add(_2, _2)
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSuite
//
// {
//     {
//         let a := 0
//         for { } a { }
//         {
//             let _1 := add(a, a)
//             let var := add(_1, _1)
//             switch a
//             case 0 { a := var }
//             default { sstore(0, var) }
//         }
//     }
// }
