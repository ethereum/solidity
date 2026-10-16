PUSH 0x0
PUSH 0x1
PUSH 0x0123456789ABCDEF
PUSH 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// ====
// EVMVersion: >=shanghai
// outputs: InputAssemblyJSON,Assembly,Bytecode,Opcodes,SourceMappings
// ----
// InputAssemblyJSON: {
//     ".code": [
//         {
//             "name": "PUSH",
//             "value": "0"
//         },
//         {
//             "name": "PUSH",
//             "value": "1"
//         },
//         {
//             "name": "PUSH",
//             "value": "0123456789ABCDEF"
//         },
//         {
//             "name": "PUSH",
//             "value": "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
//         }
//     ]
// }
// Assembly:
//   0x00
//   0x01
//   0x0123456789abcdef
//   0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// Bytecode: 5f6001670123456789abcdef7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// Opcodes: PUSH0 PUSH1 0x1 PUSH8 0x123456789ABCDEF PUSH32 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// SourceMappings: :::-:0;;;
