{
            fun_c()
            function fun_c()
            {
                switch iszero(calldataload(0))
                case 0 { }
                default {
                    if calldataload(1)
                    {
                        leave
                    }
                    if calldataload(2)
                    {
                        revert(0, 0)
                    }
                }
                revert(0, 0)
            }
}
// ====
// stackOptimization: true
// ----
//     /* "":14:21   */
//   tag_2
//   tag_1
//   jump	// in
// tag_2:
//     /* "":0:460   */
//   stop
//     /* "":34:458   */
// tag_1:
//     /* "":108:109   */
//   0x00
//     /* "":95:110   */
//   calldataload
//     /* "":88:111   */
//   iszero
//     /* "":133:134   */
//   0x00
//     /* "":128:138   */
//   eq
//   tag_3
//   jumpi
//     /* "":81:415   */
// tag_4:
//     /* "":201:202   */
//   0x01
//     /* "":188:203   */
//   calldataload
//     /* "":185:277   */
//   tag_5
//   jumpi
//     /* "":81:415   */
// tag_6:
//     /* "":301:316   */
//   pop
//     /* "":314:315   */
//   0x02
//     /* "":301:316   */
//   calldataload
//     /* "":298:397   */
//   tag_7
//   jumpi
//     /* "":81:415   */
// tag_8:
// tag_9:
//     /* "":442:443   */
//   0x00
//     /* "":432:444   */
//   dup1
//   revert
//     /* "":337:397   */
// tag_7:
//     /* "":373:374   */
//   0x00
//     /* "":363:375   */
//   dup1
//   revert
//     /* "":224:277   */
// tag_5:
//     /* "":250:255   */
//   jump	// out
//     /* "":135:138   */
// tag_3:
//   pop
//   jump(tag_9)
