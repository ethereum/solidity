{
    let a
    { }
    function f() -> x {
        let b := 4
        { }
        for {} f() {} {}
    }
}
// ----
// step: functionHoister
//
// {
//     let a
//     function f() -> x
//     {
//         let b := 4
//         for { } f() { }
//         { }
//     }
// }
