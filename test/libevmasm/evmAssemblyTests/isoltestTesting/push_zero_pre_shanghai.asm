PUSH 0x0
PUSH [$] 0x0

.sub
    PUSH 0x0
// ====
// EVMVersion: <shanghai
// outputs: InputAssemblyJSON,Assembly,Bytecode,Opcodes,SourceMappings
// ----
// InputAssemblyJSON: {
//     ".code": [
//         {
//             "name": "PUSH",
//             "value": "0"
//         },
//         {
//             "name": "PUSH [$]",
//             "value": "0"
//         }
//     ],
//     ".data": {
//         "0": {
//             ".code": [
//                 {
//                     "name": "PUSH",
//                     "value": "0"
//                 }
//             ]
//         }
//     }
// }
// Assembly:
//   0x00
//   dataOffset(sub_0)
// stop
//
// sub_0: assembly {
//       0x00
// }
// Bytecode: 60006005fe6000
// Opcodes: PUSH1 0x0 PUSH1 0x5 INVALID PUSH1 0x0
// SourceMappings: :::-:0;
