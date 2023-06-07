object "Contract" {
  code {
    function f() {}
    function g() {}
    sstore(0, 1)

    // NOTE: msize forces the compiler to completely disable the Yul optimizer.
    // Otherwise the functions would get optimized out.
    pop(msize())
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
//     /* "source":231:238   */
//   msize
//     /* "source":227:239   */
//   pop
// Bytecode: 6009565b5b565b5b565b60015f555950
// Opcodes: PUSH1 0x9 JUMP JUMPDEST JUMPDEST JUMP JUMPDEST JUMPDEST JUMP JUMPDEST PUSH1 0x1 PUSH0 SSTORE MSIZE POP
// SourceMappings: 33:15:0:-:0;;;;:::o;53:::-;;:::o;:::-;83:1;80;73:12;231:7;227:12
