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
//     /* "source":37:52   */
//   0x00
//     /* "source":68:81   */
//   bytecodeSize
//     /* "source":97:112   */
//   dataOffset(sub_0)
//     /* "source":128:141   */
//   dataSize(sub_0)
//     /* "source":158:175   */
//   dataOffset(sub_0.sub_0)
//     /* "source":192:207   */
//   dataSize(sub_0.sub_0)
//     /* "source":224:241   */
//   swap1
//   dataOffset(sub_0.sub_1)
//     /* "source":258:273   */
//   swap3
//   dataSize(sub_0.sub_1)
//     /* "source":291:310   */
//   swap5
//   dataOffset(sub_0.sub_0.sub_0)
//     /* "source":328:345   */
//   swap7
//   dataSize(sub_0.sub_0.sub_0)
//     /* "source":351:365   */
//   swap9
//     /* "source":358:359   */
//   0x00
//     /* "source":351:365   */
//   sstore
//     /* "source":377:379   */
//   0x20
//     /* "source":370:385   */
//   sstore
//     /* "source":397:399   */
//   0x40
//     /* "source":390:405   */
//   sstore
//     /* "source":417:419   */
//   0x60
//     /* "source":410:425   */
//   sstore
//     /* "source":437:440   */
//   0x80
//     /* "source":430:447   */
//   sstore
//     /* "source":459:462   */
//   0xa0
//     /* "source":452:469   */
//   sstore
//     /* "source":481:484   */
//   0xc0
//     /* "source":474:491   */
//   sstore
//     /* "source":503:506   */
//   0xe0
//     /* "source":496:513   */
//   sstore
//     /* "source":525:528   */
//   0x0100
//     /* "source":518:536   */
//   sstore
//     /* "source":548:551   */
//   0x0120
//     /* "source":541:559   */
//   sstore
//     /* "source":574:577   */
//   0x0140
//     /* "source":571:572   */
//   0x00
//     /* "source":564:578   */
//   return
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":659:674   */
//       dataOffset(sub_0)
//         /* "source":692:705   */
//       dataSize(sub_0)
//         /* "source":723:738   */
//       dataOffset(sub_1)
//         /* "source":756:769   */
//       dataSize(sub_1)
//         /* "source":788:805   */
//       swap1
//       dataOffset(sub_0.sub_0)
//         /* "source":824:839   */
//       swap3
//       dataSize(sub_0.sub_0)
//         /* "source":847:861   */
//       swap5
//         /* "source":854:855   */
//       0x00
//         /* "source":847:861   */
//       sstore
//         /* "source":875:877   */
//       0x20
//         /* "source":868:883   */
//       sstore
//         /* "source":897:899   */
//       0x40
//         /* "source":890:905   */
//       sstore
//         /* "source":919:921   */
//       0x60
//         /* "source":912:927   */
//       sstore
//         /* "source":941:944   */
//       0x80
//         /* "source":934:951   */
//       sstore
//         /* "source":965:968   */
//       0xa0
//         /* "source":958:975   */
//       sstore
//         /* "source":992:995   */
//       0xc0
//         /* "source":989:990   */
//       0x00
//         /* "source":982:996   */
//       return
//     stop
//
//     sub_0: assembly {
//             /* "source":1052:1067   */
//           dataOffset(sub_0)
//             /* "source":1087:1100   */
//           dataSize(sub_0)
//             /* "source":1110:1124   */
//           swap1
//             /* "source":1117:1118   */
//           0x00
//             /* "source":1110:1124   */
//           sstore
//             /* "source":1140:1142   */
//           0x20
//             /* "source":1133:1148   */
//           sstore
//             /* "source":1167:1169   */
//           0x40
//             /* "source":1164:1165   */
//           0x00
//             /* "source":1157:1170   */
//           return
//         stop
//
//         sub_0: assembly {
//                 /* "source":1223:1232   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":1295:1304   */
//           invalid
//     }
// }
// Bytecode: 5f6084603d603660746010906073926001946073966001985f5560205560405560605560805560a05560c05560e05561010055610120556101405ff3fe6025601060356001906035926001945f5560205560405560605560805560a05560c05ff3fe600f6001905f5560205560405ff3fefefefe600f6001905f5560205560405ff3fefe
// Opcodes: PUSH0 PUSH1 0x84 PUSH1 0x3D PUSH1 0x36 PUSH1 0x74 PUSH1 0x10 SWAP1 PUSH1 0x73 SWAP3 PUSH1 0x1 SWAP5 PUSH1 0x73 SWAP7 PUSH1 0x1 SWAP9 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 SSTORE PUSH1 0x60 SSTORE PUSH1 0x80 SSTORE PUSH1 0xA0 SSTORE PUSH1 0xC0 SSTORE PUSH1 0xE0 SSTORE PUSH2 0x100 SSTORE PUSH2 0x120 SSTORE PUSH2 0x140 PUSH0 RETURN INVALID PUSH1 0x25 PUSH1 0x10 PUSH1 0x35 PUSH1 0x1 SWAP1 PUSH1 0x35 SWAP3 PUSH1 0x1 SWAP5 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 SSTORE PUSH1 0x60 SSTORE PUSH1 0x80 SSTORE PUSH1 0xA0 SSTORE PUSH1 0xC0 PUSH0 RETURN INVALID PUSH1 0xF PUSH1 0x1 SWAP1 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 PUSH0 RETURN INVALID INVALID INVALID INVALID PUSH1 0xF PUSH1 0x1 SWAP1 PUSH0 SSTORE PUSH1 0x20 SSTORE PUSH1 0x40 PUSH0 RETURN INVALID INVALID
// SourceMappings: 37:15:0:-:0;68:13;97:15;128:13;158:17;192:15;224:17;;258:15;;291:19;;328:17;;351:14;358:1;351:14;377:2;370:15;397:2;390:15;417:2;410:15;437:3;430:17;459:3;452:17;481:3;474:17;503:3;496:17;525:3;518:18;548:3;541:18;574:3;571:1;564:14
