{
    function fun(x: i32, y) -> t: i32, z: i32 {
        z := i32.add(x, i32.add(z, z))

    }
    i64.store(i32.load(5:i32), i64.load(8:i32))
    let i := 0
    for {} i32.eqz(i32.load(9:i32)) { i := i64.add(i, 1) } {
        let f: i32, g: i32 := fun(i32.load(1:i32), i64.load(i32.load(0: i32)))
    }
}
// ====
// dialect: ewasm
// step: expressionSplitter
// ----
// {
//     function fun(x:i32, y) -> t:i32, z:i32
//     {
//         let _1:i32 := i32.add(z, z)
//         z := i32.add(x, _1)
//     }
//     let _2:i32 := 8:i32
//     let _3 := i64.load(_2)
//     let _4:i32 := 5:i32
//     let _5:i32 := i32.load(_4)
//     i64.store(_5, _3)
//     let i := 0
//     for { }
//     i32.eqz(i32.load(9:i32))
//     {
//         let _6 := 1
//         i := i64.add(i, _6)
//     }
//     {
//         let _7:i32 := 0:i32
//         let _8:i32 := i32.load(_7)
//         let _9 := i64.load(_8)
//         let _10:i32 := 1:i32
//         let _11:i32 := i32.load(_10)
//         let f:i32, g:i32 := fun(_11, _9)
//     }
// }
