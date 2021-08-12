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
//   tag_1
//     /* "":14:21   */
//   tag_2
//   jump	// in
// tag_1:
//   stop
//     /* "":34:458   */
// tag_2:
//     /* "":108:109   */
//   0x00
//     /* "":95:110   */
//   calldataload
//     /* "":88:111   */
//   iszero
//     /* "":133:134   */
//   0x00
//   eq
//   tag_3
//   jumpi
// tag_4:
//     /* "":201:202   */
//   0x01
//     /* "":188:203   */
//   calldataload
//   tag_5
//   jumpi
// tag_6:
//   pop
//     /* "":314:315   */
//   0x02
//     /* "":301:316   */
//   calldataload
//   tag_7
//   jumpi
// tag_8:
// tag_9:
//     /* "":442:443   */
//   0x00
//   dup1
//     /* "":432:444   */
//   revert
// tag_7:
//     /* "":373:374   */
//   0x00
//   dup1
//     /* "":363:375   */
//   revert
// tag_5:
//     /* "":34:458   */
//   jump	// out
// tag_3:
//   pop
//   jump(tag_9)
