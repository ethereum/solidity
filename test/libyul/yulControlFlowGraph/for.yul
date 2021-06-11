{
    sstore(0x01, 0x0101)
    for { sstore(0x02, 0x0202) } sload(0x03) { sstore(0x04, 0x0404) } {
        sstore(0x05, 0x0505)
    }
    sstore(0x06, 0x0506)
}
// ----
// Block 0:
//   Entries: None
//   sstore: [ 0x0101 0x01 ] => [ ]
//   sstore: [ 0x0202 0x02 ] => [ ]
//   Jump: 1
// Block 1:
//   Entries: 0, 2
//   sload: [ 0x03 ] => [ TMP[sload, 0] ]
//   ConditionalJump TMP[sload, 0]:
//     NonZero: 3
//     Zero: 4
// Block 2:
//   Entries: 3
//   sstore: [ 0x0404 0x04 ] => [ ]
//   Jump (backwards): 1
// Block 3:
//   Entries: 1
//   sstore: [ 0x0505 0x05 ] => [ ]
//   Jump: 2
// Block 4:
//   Entries: 1
//   sstore: [ 0x0506 0x06 ] => [ ]
//   MainExit
