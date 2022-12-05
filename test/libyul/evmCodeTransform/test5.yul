{
    {
        let i := 0
        let i_1 := i
        let _1 := mload(0x1fffffffffffffffffffffffff)
        for { } true { i_1 := add(i_1, 0x20) }
        {
            let _2 := 0x60
            if iszero(lt(i_1, _2)) { break }
            switch _1
            case 0x1ffffffffffffffffffffffffff { }
            default {
                let i_2 := i
                if lt(i, _2) { }
            }
        }
    }
}
// ====
// stackOptimization: true
// ----
//     /* "":25:26   */
//   0x00
//     /* "":35:47   */
//   dup1
//     /* "":66:101   */
//   swap1
//     /* "":72:100   */
//   0x1fffffffffffffffffffffffff
//     /* "":66:101   */
//   mload
//     /* "":114:117   */
//   swap2
//     /* "":157:411   */
// tag_1:
//     /* "":181:185   */
//   0x60
//     /* "":208:219   */
//   swap1
//   dup2
//   dup2
//   lt
//     /* "":201:220   */
//   iszero
//     /* "":198:230   */
//   tag_2
//   jumpi
//     /* "":157:411   */
// tag_3:
//     /* "":141:145   */
//   0x20
//     /* "":243:401   */
//   swap2
//   dup5
//     /* "":270:299   */
//   0x01ffffffffffffffffffffffffff
//     /* "":265:303   */
//   eq
//   tag_4
//   jumpi
//     /* "":243:401   */
// tag_5:
//     /* "":342:354   */
//   dup4
//     /* "":374:383   */
//   pop
//   dup4
//   lt
//     /* "":371:387   */
//   tag_6
//   jumpi
//     /* "":243:401   */
// tag_7:
// tag_8:
//     /* "":132:146   */
//   add
//     /* "":123:148   */
//   jump(tag_1)
//     /* "":384:387   */
// tag_6:
//   jump(tag_7)
//     /* "":300:303   */
// tag_4:
//   pop
//   jump(tag_8)
//     /* "":221:230   */
// tag_2:
//     /* "":110:411   */
//   stop
