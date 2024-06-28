{
    let a := 1234
    let a_hex := 0x4d2
    let b := "1234"
    let b_hex := "0x4d2"
    let c := '1234'
    let c_hex := '0x4d2'
    let d := hex"1234"
    let d_hex := hex"04d2"
}
// ----
// step: disambiguator
//
// {
//     let a := 1234
//     let a_hex := 0x4d2
//     let b := "1234"
//     let b_hex := "0x4d2"
//     let c := "1234"
//     let c_hex := "0x4d2"
//     let d := "\x124"
//     let d_hex := "\x04\xd2"
// }
