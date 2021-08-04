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
// ----
// Assembly:
//     /* "source":35:48   */
//   0x0e
//     /* "source":62:75   */
//   0x03
//     /* "source":90:91   */
//   dup2
//     /* "source":87:88   */
//   0x00
//     /* "source":80:92   */
//   sstore
//     /* "source":108:109   */
//   dup1
//     /* "source":104:106   */
//   0x20
//     /* "source":97:110   */
//   sstore
//     /* "source":20:114   */
//   pop
//   pop
// stop
// data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
// data_e1629b9dda060bb30c7908346f6af189c16773fa148d3366701fbaa35d54f3c8 414243
//
// sub_0: assembly {
//         /* "source":157:176   */
//       data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336
//         /* "source":193:194   */
//       dup1
//         /* "source":190:191   */
//       0x00
//         /* "source":183:195   */
//       sstore
//         /* "source":140:201   */
//       pop
//     stop
//     data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
//
//     auxdata: 0x4d31
// }
//
// auxdata: 0x4d32
// Bytecode: 600e600381600055806020555050fe4d32
// Opcodes: PUSH1 0xE PUSH1 0x3 DUP2 PUSH1 0x0 SSTORE DUP1 PUSH1 0x20 SSTORE POP POP INVALID 0x4D ORIGIN
// SourceMappings: 35:13:0:-:0;62;90:1;87;80:12;108:1;104:2;97:13;20:94;
