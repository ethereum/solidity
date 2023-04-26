object "Contract" {
  code {
    function f() {}
    function g() {}
    sstore(0, 1)
  }
}

// ====
// optimizationPreset: none
// ----
// Assembly:
//     /* "source":33:48   */
//   jump(tag_3)
// tag_1:
// tag_4:
//   jump	// out
//     /* "source":53:68   */
// tag_2:
// tag_5:
//   jump	// out
// tag_3:
//     /* "source":83:84   */
//   0x01
//     /* "source":80:81   */
//   0x00
//     /* "source":73:85   */
//   sstore
// Bytecode: 6009565b5b565b5b565b60015f55
// Opcodes: PUSH1 0x9 JUMP JUMPDEST JUMPDEST JUMP JUMPDEST JUMPDEST JUMP JUMPDEST PUSH1 0x1 PUSH0 SSTORE
// SourceMappings: 33:15:0:-:0;;;;:::o;53:::-;;:::o;:::-;83:1;80;73:12
