{
    let x := 7

    let y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, y13, y14, y15 := verbatim_0i_15o("\x60\x42") // the verbatim will show up as PUSH1 42

    // last use of x - the slot of x will be marked as unused, but not popped, since it is not at the stack top
    sstore(0,x)

    // If the slot of x is blindly reused, this will fail.
    let z1, z2 := verbatim_0i_2o("\x60\x43") // will show up as PUSH1 43

    // prevent the z's from being popped immediately after their declaration above.
    mstore(1, z1)
    mstore(1, z2)

    // use all y's to prevent them from being popped immediately after their declaration above
    sstore(1, y1)
    sstore(1, y2)
    sstore(1, y3)
    sstore(1, y4)
    sstore(1, y5)
    sstore(1, y6)
    sstore(1, y7)
    sstore(1, y8)
    sstore(1, y9)
    sstore(1, y10)
    sstore(1, y11)
    sstore(1, y12)
    sstore(1, y13)
    sstore(1, y14)
    sstore(1, y15)
}
// ====
// stackOptimization: true
// ----
//     /* "":15:16   */
//   0x07
//     /* "":94:121   */
//   verbatimbytecode_6042
//   swap15
//     /* "":287:288   */
//   0x00
//   swap15
//   swap2
//   swap15
//   swap14
//   swap3
//   swap14
//   swap13
//   swap4
//   swap13
//   swap12
//   swap5
//   swap12
//   swap11
//   swap6
//   swap11
//   swap10
//   swap7
//   swap10
//   swap9
//   swap8
//   swap9
//     /* "":280:291   */
//   sstore
//     /* "":370:396   */
//   verbatimbytecode_6043
//   swap1
//     /* "":521:522   */
//   0x01
//     /* "":514:527   */
//   mstore
//     /* "":539:540   */
//   0x01
//     /* "":532:545   */
//   mstore
//     /* "":653:654   */
//   0x01
//     /* "":646:659   */
//   sstore
//     /* "":671:672   */
//   0x01
//     /* "":664:677   */
//   sstore
//     /* "":689:690   */
//   0x01
//     /* "":682:695   */
//   sstore
//     /* "":707:708   */
//   0x01
//     /* "":700:713   */
//   sstore
//     /* "":725:726   */
//   0x01
//     /* "":718:731   */
//   sstore
//     /* "":743:744   */
//   0x01
//     /* "":736:749   */
//   sstore
//     /* "":761:762   */
//   0x01
//     /* "":754:767   */
//   sstore
//     /* "":779:780   */
//   0x01
//     /* "":772:785   */
//   sstore
//     /* "":797:798   */
//   0x01
//     /* "":790:803   */
//   sstore
//     /* "":815:816   */
//   0x01
//     /* "":808:822   */
//   sstore
//     /* "":834:835   */
//   0x01
//     /* "":827:841   */
//   sstore
//     /* "":853:854   */
//   0x01
//     /* "":846:860   */
//   sstore
//     /* "":872:873   */
//   0x01
//     /* "":865:879   */
//   sstore
//     /* "":891:892   */
//   0x01
//     /* "":884:898   */
//   sstore
//     /* "":910:911   */
//   0x01
//     /* "":903:917   */
//   sstore
//   stop
