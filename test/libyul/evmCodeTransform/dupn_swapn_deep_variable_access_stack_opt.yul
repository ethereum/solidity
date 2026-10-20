{
    let a1 := calldataload(0x0)
    let a2 := calldataload(0x20)
    let a3 := calldataload(0x40)
    let a4 := calldataload(0x60)
    let a5 := calldataload(0x80)
    let a6 := calldataload(0xa0)
    let a7 := calldataload(0xc0)
    let a8 := calldataload(0xe0)
    let a9 := calldataload(0x100)
    let a10 := calldataload(0x120)
    let a11 := calldataload(0x140)
    let a12 := calldataload(0x160)
    let a13 := calldataload(0x180)
    let a14 := calldataload(0x1a0)
    let a15 := calldataload(0x1c0)
    let a16 := calldataload(0x1e0)
    let a17 := calldataload(0x200)
    let a18 := calldataload(0x220)
    let a19 := calldataload(0x240)
    let a20 := calldataload(0x260)
    let a21 := calldataload(0x280)
    let a22 := calldataload(0x2a0)
    let a23 := calldataload(0x2c0)
    let a24 := calldataload(0x2e0)
    sstore(1, a1)
    sstore(2, a2)
    sstore(3, a3)
    sstore(4, a4)
    sstore(5, a5)
    sstore(6, a6)
    sstore(7, a7)
    sstore(8, a8)
    sstore(9, a9)
    sstore(10, a10)
    sstore(11, a11)
    sstore(12, a12)
    sstore(13, a13)
    sstore(14, a14)
    sstore(15, a15)
    sstore(16, a16)
    sstore(17, a17)
    sstore(18, a18)
    sstore(19, a19)
    sstore(20, a20)
    sstore(21, a21)
    sstore(22, a22)
    sstore(23, a23)
    sstore(24, a24)
    sstore(0, add(a1, a24))
}
// ====
// EVMVersion: >=amsterdam
// stackOptimization: true
// ----
//     /* "":29:32   */
//   0x00
//     /* "":16:33   */
//   calldataload
//     /* "":61:65   */
//   0x20
//     /* "":48:66   */
//   calldataload
//     /* "":81:99   */
//   swap1
//     /* "":94:98   */
//   0x40
//     /* "":81:99   */
//   calldataload
//     /* "":127:131   */
//   0x60
//     /* "":114:132   */
//   calldataload
//     /* "":160:164   */
//   0x80
//     /* "":147:165   */
//   calldataload
//     /* "":193:197   */
//   0xa0
//     /* "":180:198   */
//   calldataload
//     /* "":226:230   */
//   0xc0
//     /* "":213:231   */
//   calldataload
//     /* "":259:263   */
//   0xe0
//     /* "":246:264   */
//   calldataload
//     /* "":292:297   */
//   0x0100
//     /* "":279:298   */
//   calldataload
//     /* "":327:332   */
//   0x0120
//     /* "":314:333   */
//   calldataload
//     /* "":362:367   */
//   0x0140
//     /* "":349:368   */
//   calldataload
//     /* "":397:402   */
//   0x0160
//     /* "":384:403   */
//   calldataload
//     /* "":432:437   */
//   0x0180
//     /* "":419:438   */
//   calldataload
//     /* "":467:472   */
//   0x01a0
//     /* "":454:473   */
//   calldataload
//     /* "":489:508   */
//   swap2
//     /* "":502:507   */
//   0x01c0
//     /* "":489:508   */
//   calldataload
//     /* "":524:543   */
//   swap4
//     /* "":537:542   */
//   0x01e0
//     /* "":524:543   */
//   calldataload
//     /* "":559:578   */
//   swap6
//     /* "":572:577   */
//   0x0200
//     /* "":559:578   */
//   calldataload
//     /* "":594:613   */
//   swap8
//     /* "":607:612   */
//   0x0220
//     /* "":594:613   */
//   calldataload
//     /* "":629:648   */
//   swap10
//     /* "":642:647   */
//   0x0240
//     /* "":629:648   */
//   calldataload
//     /* "":664:683   */
//   swap12
//     /* "":677:682   */
//   0x0260
//     /* "":664:683   */
//   calldataload
//     /* "":699:718   */
//   swap14
//     /* "":712:717   */
//   0x0280
//     /* "":699:718   */
//   calldataload
//     /* "":734:753   */
//   swap16
//     /* "":747:752   */
//   0x02a0
//     /* "":734:753   */
//   calldataload
//     /* "":769:788   */
//   swapn{18}
//     /* "":782:787   */
//   0x02c0
//     /* "":769:788   */
//   calldataload
//     /* "":804:823   */
//   swapn{20}
//     /* "":817:822   */
//   0x02e0
//     /* "":804:823   */
//   calldataload
//     /* "":828:841   */
//   swapn{23}
//   dupn{23}
//     /* "":835:836   */
//   0x01
//     /* "":828:841   */
//   sstore
//     /* "":853:854   */
//   0x02
//     /* "":846:859   */
//   sstore
//     /* "":871:872   */
//   0x03
//     /* "":864:877   */
//   sstore
//     /* "":889:890   */
//   0x04
//     /* "":882:895   */
//   sstore
//     /* "":907:908   */
//   0x05
//     /* "":900:913   */
//   sstore
//     /* "":925:926   */
//   0x06
//     /* "":918:931   */
//   sstore
//     /* "":943:944   */
//   0x07
//     /* "":936:949   */
//   sstore
//     /* "":961:962   */
//   0x08
//     /* "":954:967   */
//   sstore
//     /* "":979:980   */
//   0x09
//     /* "":972:985   */
//   sstore
//     /* "":997:999   */
//   0x0a
//     /* "":990:1005   */
//   sstore
//     /* "":1017:1019   */
//   0x0b
//     /* "":1010:1025   */
//   sstore
//     /* "":1037:1039   */
//   0x0c
//     /* "":1030:1045   */
//   sstore
//     /* "":1057:1059   */
//   0x0d
//     /* "":1050:1065   */
//   sstore
//     /* "":1077:1079   */
//   0x0e
//     /* "":1070:1085   */
//   sstore
//     /* "":1097:1099   */
//   0x0f
//     /* "":1090:1105   */
//   sstore
//     /* "":1117:1119   */
//   0x10
//     /* "":1110:1125   */
//   sstore
//     /* "":1137:1139   */
//   0x11
//     /* "":1130:1145   */
//   sstore
//     /* "":1157:1159   */
//   0x12
//     /* "":1150:1165   */
//   sstore
//     /* "":1177:1179   */
//   0x13
//     /* "":1170:1185   */
//   sstore
//     /* "":1197:1199   */
//   0x14
//     /* "":1190:1205   */
//   sstore
//     /* "":1217:1219   */
//   0x15
//     /* "":1210:1225   */
//   sstore
//     /* "":1237:1239   */
//   0x16
//     /* "":1230:1245   */
//   sstore
//     /* "":1257:1259   */
//   0x17
//     /* "":1250:1265   */
//   sstore
//     /* "":1270:1285   */
//   dup2
//     /* "":1277:1279   */
//   0x18
//     /* "":1270:1285   */
//   sstore
//     /* "":1300:1312   */
//   add
//     /* "":1297:1298   */
//   0x00
//     /* "":1290:1313   */
//   sstore
//     /* "":0:1315   */
//   stop
