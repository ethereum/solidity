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
// stackOptimization: true
// ----
//     /* "":23:24   */
//   0x00
//     /* "":27:40   */
// tag_1:
//     /* "":35:39   */
//   0x60
//     /* "":27:40   */
//   dup2
//   lt
//   tag_2
//   jumpi
//     /* "":6:527   */
// tag_3:
//   stop
//     /* "":71:527   */
// tag_2:
//     /* "":59:63   */
//   0x20
//     /* "":88:123   */
//   swap1
//     /* "":94:122   */
//   0x1fffffffffffffffffffffffff
//     /* "":88:123   */
//   mload
//     /* "":137:166   */
//   0x01ffffffffffffffffffffffffff
//     /* "":132:170   */
//   eq
//   tag_4
//   jumpi
//     /* "":81:500   */
// tag_5:
//     /* "":218:219   */
//   0x00
//     /* "":222:235   */
// tag_6:
//     /* "":230:234   */
//   0x60
//     /* "":222:235   */
//   dup2
//   lt
//   tag_7
//   jumpi
//     /* "":201:490   */
// tag_8:
//     /* "":187:500   */
//   pop
//     /* "":81:500   */
// tag_9:
//     /* "":509:521   */
//   0x00
//   dup1
//     /* "":71:527   */
//   pop
//   pop
//     /* "":50:64   */
//   add
//     /* "":41:66   */
//   jump(tag_1)
//     /* "":274:490   */
// tag_7:
//     /* "":299:329   */
//   0x1fffffffffffffffffffffffffff
//     /* "":353:384   */
//   0x01ffffffffffffffffffffffffffff
//     /* "":346:388   */
//   eq
//   tag_10
//   jumpi
//     /* "":292:422   */
// tag_11:
//     /* "":415:420   */
//   jump(tag_8)
//     /* "":385:388   */
// tag_10:
//   dup3
//   swap1
//     /* "":245:259   */
//   add
//     /* "":236:261   */
//   jump(tag_6)
//     /* "":167:170   */
// tag_4:
//   jump(tag_9)
