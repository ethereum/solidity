object "A" {
  code {
    let x := datasize("x")
    let c := datasize("C")
    sstore(0, x)
    sstore(32, c)
  }

  object "B" {
    code {
      let o := dataoffset("other")
      sstore(0, o)
    }
    data ".metadata" "M1"
    data "other" "Hello, World2!"
  }

  data "C" "ABC"
  data ".metadata" "M2"
  data "x" "Hello, World2!"
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":55:68   */
//   0x0e
//     /* "source":90:103   */
//   0x03
//     /* "source":116:128   */
//   swap1
//     /* "source":123:124   */
//   0x00
//     /* "source":116:128   */
//   sstore
//     /* "source":148:150   */
//   0x20
//     /* "source":141:154   */
//   sstore
//     /* "source":22:170   */
//   stop
// stop
// data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
// data_e1629b9dda060bb30c7908346f6af189c16773fa148d3366701fbaa35d54f3c8 414243
//
// sub_0: assembly {
//         /* "source":242:261   */
//       data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336
//         /* "source":285:286   */
//       0x00
//         /* "source":278:290   */
//       sstore
//         /* "source":201:314   */
//       stop
//     stop
//     data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
//
//     auxdata: 0x4d31
// }
//
// auxdata: 0x4d32
// Bytecode: 600e6003905f5560205500fe4d32
// Opcodes: PUSH1 0xE PUSH1 0x3 SWAP1 PUSH0 SSTORE PUSH1 0x20 SSTORE STOP INVALID 0x4D ORIGIN
// SourceMappings: 55:13:0:-:0;90;116:12;123:1;116:12;148:2;141:13;22:148
