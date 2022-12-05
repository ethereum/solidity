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
// ----
//     /* "":25:26   */
//   0x00
//     /* "":46:47   */
//   dup1
//     /* "":72:100   */
//   0x1fffffffffffffffffffffffff
//     /* "":66:101   */
//   mload
//     /* "":110:411   */
// tag_1:
//     /* "":118:122   */
//   0x01
//     /* "":110:411   */
//   iszero
//   tag_3
//   jumpi
//     /* "":181:185   */
//   0x60
//     /* "":216:218   */
//   dup1
//     /* "":211:214   */
//   dup4
//     /* "":208:219   */
//   lt
//     /* "":201:220   */
//   iszero
//     /* "":198:230   */
//   iszero
//   tag_4
//   jumpi
//     /* "":223:228   */
//   pop
//   jump(tag_3)
//     /* "":198:230   */
// tag_4:
//     /* "":250:252   */
//   dup2
//     /* "":270:299   */
//   0x01ffffffffffffffffffffffffff
//     /* "":265:303   */
//   dup2
//   eq
//   tag_6
//   jumpi
//     /* "":353:354   */
//   dup5
//     /* "":380:382   */
//   dup3
//     /* "":377:378   */
//   dup7
//     /* "":374:383   */
//   lt
//     /* "":371:387   */
//   iszero
//   tag_7
//   jumpi
// tag_7:
//     /* "":324:401   */
//   pop
//     /* "":243:401   */
//   jump(tag_5)
//     /* "":265:303   */
// tag_6:
//     /* "":243:401   */
// tag_5:
//   pop
//     /* "":157:411   */
//   pop
//     /* "":110:411   */
// tag_2:
//     /* "":141:145   */
//   0x20
//     /* "":136:139   */
//   dup3
//     /* "":132:146   */
//   add
//     /* "":125:146   */
//   swap2
//   pop
//     /* "":110:411   */
//   jump(tag_1)
// tag_3:
//     /* "":6:417   */
//   pop
//   pop
//   pop
