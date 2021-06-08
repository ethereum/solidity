object "A" {
  code {
    pop(datasize("x"))
    pop(datasize("C"))
  }

  object "B" {
    code { pop(dataoffset("other")) }
    data ".metadata" "M1"
    data "other" "Hello, World2!"
  }

  data "C" "ABC"
  data ".metadata" "M2"
  data "x" "Hello, World2!"
}
// ----
// Assembly:
//     /* "source":26:44   */
//   pop(0x0e)
//     /* "source":49:67   */
//   pop(0x03)
// stop
// data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
// data_e1629b9dda060bb30c7908346f6af189c16773fa148d3366701fbaa35d54f3c8 414243
//
// sub_0: assembly {
//         /* "source":99:123   */
//       pop(data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336)
//     stop
//     data_211450822d7f8c345093893187e7e1fbebc4ec67af72601920194be14104e336 48656c6c6f2c20576f726c643221
//
//     auxdata: 0x4d31
// }
//
// auxdata: 0x4d32
// Bytecode: 600e50600350fe4d32
// Opcodes: PUSH1 0xE POP PUSH1 0x3 POP INVALID 0x4D ORIGIN
// SourceMappings: 26:18:0:-:0;;49;
