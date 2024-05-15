object "A" {
    code {
        pop(datasize(hex'616263'))
    }
    data 'abc' "1234"
}
// ----
// step: disambiguator
//
// object "A" {
//     code { pop(datasize("abc")) }
//     data "abc" hex"31323334"
// }
