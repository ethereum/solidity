{
    for { let i_0 := 0 } lt(i_0, 0x60) { i_0 := add(i_0, 0x20) }
    {
        switch mload(0x1fffffffffffffffffffffffff)
        case 0x1ffffffffffffffffffffffffff { }
        default {
            for { let i_1 := 0 } lt(i_1, 0x60) { i_1 := add(i_1, 0x20) }
            {
                switch 0x1fffffffffffffffffffffffffff
                case   0x1ffffffffffffffffffffffffffff { }
                default { break }
                continue
                let x_4, x_5
            }
        }
        let x_6, x_7
    }
}
// ====
// stackOptimization: false
// ----
//     /* "":23:24   */
//   0x00
//     /* "":6:527   */
// tag_1:
//     /* "":35:39   */
//   0x60
//     /* "":30:33   */
//   dup2
//     /* "":27:40   */
//   lt
//     /* "":6:527   */
//   iszero
//   tag_3
//   jumpi
//     /* "":94:122   */
//   0x1fffffffffffffffffffffffff
//     /* "":88:123   */
//   mload
//     /* "":137:166   */
//   0x01ffffffffffffffffffffffffff
//     /* "":132:170   */
//   dup2
//   eq
//   tag_5
//   jumpi
//     /* "":218:219   */
//   0x00
//     /* "":201:490   */
// tag_6:
//     /* "":230:234   */
//   0x60
//     /* "":225:228   */
//   dup2
//     /* "":222:235   */
//   lt
//     /* "":201:490   */
//   iszero
//   tag_8
//   jumpi
//     /* "":299:329   */
//   0x1fffffffffffffffffffffffffff
//     /* "":353:384   */
//   0x01ffffffffffffffffffffffffffff
//     /* "":346:388   */
//   dup2
//   eq
//   tag_10
//   jumpi
//     /* "":415:420   */
//   pop
//   jump(tag_8)
//     /* "":292:422   */
//   jump(tag_9)
//     /* "":346:388   */
// tag_10:
//     /* "":292:422   */
// tag_9:
//   pop
//     /* "":439:447   */
//   jump(tag_7)
//     /* "":464:476   */
//   0x00
//   0x00
//     /* "":274:490   */
//   pop
//   pop
//     /* "":201:490   */
// tag_7:
//     /* "":254:258   */
//   0x20
//     /* "":249:252   */
//   dup2
//     /* "":245:259   */
//   add
//     /* "":238:259   */
//   swap1
//   pop
//     /* "":201:490   */
//   jump(tag_6)
// tag_8:
//     /* "":205:221   */
//   pop
//     /* "":81:500   */
//   jump(tag_4)
//     /* "":132:170   */
// tag_5:
//     /* "":81:500   */
// tag_4:
//   pop
//     /* "":509:521   */
//   0x00
//   0x00
//     /* "":71:527   */
//   pop
//   pop
//     /* "":6:527   */
// tag_2:
//     /* "":59:63   */
//   0x20
//     /* "":54:57   */
//   dup2
//     /* "":50:64   */
//   add
//     /* "":43:64   */
//   swap1
//   pop
//     /* "":6:527   */
//   jump(tag_1)
// tag_3:
//     /* "":10:26   */
//   pop
