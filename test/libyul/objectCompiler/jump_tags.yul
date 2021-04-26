object "Contract" {
  code {
    function f() { g(1) }
    function g(x) { if x { leave } g(add(x, 2)) }
    g(1)
  }
}

// ====
// optimizationPreset: none
// ----
// Assembly:
//     /* "source":33:54   */
//   jump(tag_1)
// tag_2:
//     /* "source":48:52   */
//   tag_4
//     /* "source":50:51   */
//   0x01
//     /* "source":48:52   */
//   tag_5
//   jump	// in
// tag_4:
//     /* "source":46:54   */
// tag_3:
//   jump	// out
//     /* "source":59:104   */
// tag_5:
//     /* "source":78:79   */
//   dup1
//     /* "source":75:77   */
//   iszero
//   tag_7
//   jumpi
//     /* "source":82:87   */
//   pop
//   jump(tag_6)
//     /* "source":75:77   */
// tag_7:
//     /* "source":90:102   */
//   tag_8
//     /* "source":99:100   */
//   0x02
//     /* "source":96:97   */
//   dup3
//     /* "source":92:101   */
//   add
//     /* "source":90:102   */
//   tag_5
//   jump	// in
// tag_8:
//     /* "source":73:104   */
//   pop
// tag_6:
//   jump	// out
// tag_1:
//     /* "source":109:113   */
//   tag_9
//     /* "source":111:112   */
//   0x01
//     /* "source":109:113   */
//   tag_5
//   jump	// in
// tag_9:
// Bytecode: 6026565b600b6001600e565b5b565b8015601857506024565b602260028201600e565b505b565b602e6001600e565b
// Opcodes: PUSH1 0x26 JUMP JUMPDEST PUSH1 0xB PUSH1 0x1 PUSH1 0xE JUMP JUMPDEST JUMPDEST JUMP JUMPDEST DUP1 ISZERO PUSH1 0x18 JUMPI POP PUSH1 0x24 JUMP JUMPDEST PUSH1 0x22 PUSH1 0x2 DUP3 ADD PUSH1 0xE JUMP JUMPDEST POP JUMPDEST JUMP JUMPDEST PUSH1 0x2E PUSH1 0x1 PUSH1 0xE JUMP JUMPDEST
// SourceMappings: 33:21:0:-:0;;;48:4;50:1;48:4;:::i;:::-;46:8;:::o;59:45::-;78:1;75:2;;;82:5;;;75:2;90:12;99:1;96;92:9;90:12;:::i;:::-;73:31;;:::o;:::-;109:4;111:1;109:4;:::i;:::-
