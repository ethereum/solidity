{
    let x := illegal4(1, 2)
    function illegal4(illegal1, illegal2) -> illegal3 { illegal3 := add(illegal1, illegal2) }
    {
        let y := illegal5(3, 4)
        function illegal5(illegal1, illegal2) -> illegal3 { illegal3 := add(illegal1, illegal2) }
    }
}
// ----
// step: nameDisplacer
//
// {
//     let x := illegal4_1(1, 2)
//     function illegal4_1(illegal1_1, illegal2_1) -> illegal3_1
//     {
//         illegal3_1 := add(illegal1_1, illegal2_1)
//     }
//     {
//         let y := illegal5_1(3, 4)
//         function illegal5_1(illegal1_2, illegal2_2) -> illegal3_2
//         {
//             illegal3_2 := add(illegal1_2, illegal2_2)
//         }
//     }
// }
