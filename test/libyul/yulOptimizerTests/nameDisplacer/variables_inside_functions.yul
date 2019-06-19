{
    function f(illegal1, illegal2) -> illegal3 {
        let illegal4 := illegal1
        illegal3 := add(illegal1, illegal2)
    }
}
// ====
// step: nameDisplacer
// ----
// {
//     function f(illegal1_1, illegal2_2) -> illegal3_3
//     {
//         let illegal4_4 := illegal1_1
//         illegal3_3 := add(illegal1_1, illegal2_2)
//     }
// }
