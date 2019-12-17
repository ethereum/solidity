object "Contract" {
  code {
    function f() {}
    function g() {}
    sstore(0, 1)
  }
}

// ----
// Assembly:
//     /* "source":33:48   */
//   jump(tag_1)
// tag_2:
//     /* "source":46:48   */
// tag_3:
//   jump
//     /* "source":53:68   */
// tag_4:
//     /* "source":66:68   */
// tag_5:
//   jump
// tag_1:
//     /* "source":83:84   */
//   0x01
//     /* "source":80:81   */
//   0x00
//     /* "source":73:85   */
//   sstore
// Bytecode: 6009565b5b565b5b565b6001600055
// Opcodes: PUSH1 0x9 JUMP JUMPDEST JUMPDEST JUMP JUMPDEST JUMPDEST JUMP JUMPDEST PUSH1 0x1 PUSH1 0x0 SSTORE
