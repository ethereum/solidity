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
//     /* "source":53:54   */
//   0x02
//     /* "source":108:140   */
// tag_1:
//     /* "source":136:137   */
//   0x00
//     /* "source":134:138   */
//   tag_1
//   jump	// in
//     /* "source":149:181   */
// tag_2:
//     /* "source":177:178   */
//   0x00
//     /* "source":175:179   */
//   tag_2
//   jump	// in
//     /* "source":190:222   */
// tag_3:
//     /* "source":218:219   */
//   0x00
//     /* "source":216:220   */
//   tag_3
//   jump	// in
// Bytecode: 60025b5f6002565b5f6007565b5f600c56
// Opcodes: PUSH1 0x2 JUMPDEST PUSH0 PUSH1 0x2 JUMP JUMPDEST PUSH0 PUSH1 0x7 JUMP JUMPDEST PUSH0 PUSH1 0xC JUMP
// SourceMappings: 53:1:0:-:0;108:32;136:1;134:4;:::i;149:32::-;177:1;175:4;:::i;190:32::-;218:1;216:4;:::i
