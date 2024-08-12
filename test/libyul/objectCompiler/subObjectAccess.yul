object "A" {
  code {
    let a_o := dataoffset("A")
    let a_s := datasize("A")
    let b_o := dataoffset("B")
    let b_s := datasize("B")
    let bc_o := dataoffset("B.C")
    let bc_s := datasize("B.C")
    let be_o := dataoffset("B.E")
    let be_s := datasize("B.E")
    let bcd_o := dataoffset("B.C.D")
    let bcd_s := datasize("B.C.D")

    sstore(0, a_o)
    sstore(32, a_s)
    sstore(64, b_o)
    sstore(96, b_s)
    sstore(128, bc_o)
    sstore(160, bc_s)
    sstore(192, be_o)
    sstore(224, be_s)
    sstore(256, bcd_o)
    sstore(288, bcd_s)
    return(0, 320)
  }

  data "data1" "Hello, World!"

  object "B" {
    code {
      let c_o := dataoffset("C")
      let c_s := datasize("C")
      let e_o := dataoffset("E")
      let e_s := datasize("E")
      let cd_o := dataoffset("C.D")
      let cd_s := datasize("C.D")

      sstore(0, c_o)
      sstore(32, c_s)
      sstore(64, e_o)
      sstore(96, e_s)
      sstore(128, cd_o)
      sstore(160, cd_s)
      return(0, 192)
    }
    object "C" {
      code {
        let d_o := dataoffset("D")
        let d_s := datasize("D")

        sstore(0, d_o)
        sstore(32, d_s)
        return(0, 64)
      }
      object "D" {
        code {
          invalid()
        }
      }
    }
    object "E" {
      code {
        invalid()
      }
    }
  }
}
// ----
// Assembly:
//     /* "source":57:72   */
//   0x00
//     /* "source":96:109   */
//   bytecodeSize
//     /* "source":133:148   */
//   dataOffset(sub_0)
//     /* "source":172:185   */
//   dataSize(sub_0)
//     /* "source":210:227   */
//   dataOffset(sub_0.sub_0)
//     /* "source":252:267   */
//   dataSize(sub_0.sub_0)
//     /* "source":292:309   */
//   swap1
//   dataOffset(sub_0.sub_1)
//     /* "source":334:349   */
//   swap3
//   dataSize(sub_0.sub_1)
//     /* "source":375:394   */
//   swap5
//   dataOffset(sub_0.sub_0.sub_0)
//     /* "source":420:437   */
//   swap7
//   dataSize(sub_0.sub_0.sub_0)
//     /* "source":450:464   */
//   swap9
//     /* "source":457:458   */
//   0x00
//     /* "source":450:464   */
//   sstore
//     /* "source":484:486   */
//   0x20
//     /* "source":477:492   */
//   sstore
//     /* "source":512:514   */
//   0x40
//     /* "source":505:520   */
//   sstore
//     /* "source":540:542   */
//   0x60
//     /* "source":533:548   */
//   sstore
//     /* "source":568:571   */
//   0x80
//     /* "source":561:578   */
//   sstore
//     /* "source":598:601   */
//   0xa0
//     /* "source":591:608   */
//   sstore
//     /* "source":628:631   */
//   0xc0
//     /* "source":621:638   */
//   sstore
//     /* "source":658:661   */
//   0xe0
//     /* "source":651:668   */
//   sstore
//     /* "source":688:691   */
//   0x0100
//     /* "source":681:699   */
//   sstore
//     /* "source":719:722   */
//   0x0120
//     /* "source":712:730   */
//   sstore
//     /* "source":753:756   */
//   0x0140
//     /* "source":750:751   */
//   0x00
//     /* "source":743:757   */
//   return
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":896:911   */
//       dataOffset(sub_0)
//         /* "source":939:952   */
//       dataSize(sub_0)
//         /* "source":980:995   */
//       dataOffset(sub_1)
//         /* "source":1023:1036   */
//       dataSize(sub_1)
//         /* "source":1065:1082   */
//       swap1
//       dataOffset(sub_0.sub_0)
//         /* "source":1111:1126   */
//       swap3
//       dataSize(sub_0.sub_0)
//         /* "source":1143:1157   */
//       swap5
//         /* "source":1150:1151   */
//       0x00
//         /* "source":1143:1157   */
//       sstore
//         /* "source":1181:1183   */
//       0x20
//         /* "source":1174:1189   */
//       sstore
//         /* "source":1213:1215   */
//       0x40
//         /* "source":1206:1221   */
//       sstore
//         /* "source":1245:1247   */
//       0x60
//         /* "source":1238:1253   */
//       sstore
//         /* "source":1277:1280   */
//       0x80
//         /* "source":1270:1287   */
//       sstore
//         /* "source":1311:1314   */
//       0xa0
//         /* "source":1304:1321   */
//       sstore
//         /* "source":1348:1351   */
//       0xc0
//         /* "source":1345:1346   */
//       0x00
//         /* "source":1338:1352   */
//       return
//     stop
//
//     sub_0: assembly {
//             /* "source":1466:1481   */
//           dataOffset(sub_0)
//             /* "source":1513:1526   */
//           dataSize(sub_0)
//             /* "source":1547:1561   */
//           swap1
//             /* "source":1554:1555   */
//           0x00
//             /* "source":1547:1561   */
//           sstore
//             /* "source":1589:1591   */
//           0x20
//             /* "source":1582:1597   */
//           sstore
//             /* "source":1628:1630   */
//           0x40
//             /* "source":1625:1626   */
//           0x00
//             /* "source":1618:1631   */
//           return
//         stop
//
//         sub_0: assembly {
//                 /* "source":1714:1723   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":1794:1803   */
//           invalid
//     }
// }
// Bytecode: 5f6084603d603660746010906073926001946073966001985f5560205560405560605560805560a05560c05560e05561010055610120556101405ff3fe6025601060356001906035926001945f5560205560405560605560805560a05560c05ff3fe600f6001905f5560205560405ff3fefefefe600f6001905f5560205560405ff3fefe
// Opcodes: PUSH0 PUSH1 0x84 PUSH1 0x3D PUSH1 0x36 PUSH1 0x74 PUSH1 0x10 SWAP1 PUSH1 0x73 SWAP3 PUSH1 0x1 SWAP5 PUSH1 0x73 SWAP7 PUSH1 0x1 SWAP9 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 SSTORE PUSH1 0x60 SSTORE PUSH1 0x80 SSTORE PUSH1 0xA0 SSTORE PUSH1 0xC0 SSTORE PUSH1 0xE0 SSTORE PUSH2 0x100 SSTORE PUSH2 0x120 SSTORE PUSH2 0x140 PUSH0 RETURN INVALID PUSH1 0x25 PUSH1 0x10 PUSH1 0x35 PUSH1 0x1 SWAP1 PUSH1 0x35 SWAP3 PUSH1 0x1 SWAP5 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 SSTORE PUSH1 0x60 SSTORE PUSH1 0x80 SSTORE PUSH1 0xA0 SSTORE PUSH1 0xC0 PUSH0 RETURN INVALID PUSH1 0xF PUSH1 0x1 SWAP1 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 PUSH0 RETURN INVALID INVALID INVALID INVALID PUSH1 0xF PUSH1 0x1 SWAP1 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 PUSH0 RETURN INVALID INVALID
// SourceMappings: 57:15:0:-:0;96:13;133:15;172:13;210:17;252:15;292:17;;334:15;;375:19;;420:17;;450:14;457:1;450:14;484:2;477:15;512:2;505:15;540:2;533:15;568:3;561:17;598:3;591:17;628:3;621:17;658:3;651:17;688:3;681:18;719:3;712:18;753:3;750:1;743:14
