{
    function e(_._) {
        e(0)
    }
    e(2)
    function f(n._) {
        f(0)
    }
    f(2)
    function g(_.n) {
        g(0)
    }
    g(2)
}
// ----
// Assembly:
//     /* "source":49:50   */
//   0x02
//     /* "source":6:42   */
// tag_1:
//     /* "source":34:35   */
//   0x00
//     /* "source":32:36   */
//   tag_1
//   jump	// in
//     /* "source":56:92   */
// tag_2:
//     /* "source":84:85   */
//   0x00
//     /* "source":82:86   */
//   tag_2
//   jump	// in
//     /* "source":106:142   */
// tag_3:
//     /* "source":134:135   */
//   0x00
//     /* "source":132:136   */
//   tag_3
//   jump	// in
// Bytecode: 60025b5f6002565b5f6007565b5f600c56
// Opcodes: PUSH1 0x2 JUMPDEST PUSH0 PUSH1 0x2 JUMP JUMPDEST PUSH0 PUSH1 0x7 JUMP JUMPDEST PUSH0 PUSH1 0xC JUMP
// SourceMappings: 49:1:0:-:0;6:36;34:1;32:4;:::i;56:36::-;84:1;82:4;:::i;106:36::-;134:1;132:4;:::i
