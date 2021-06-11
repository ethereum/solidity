{
    sstore(0x01, 0x0101)
    if calldataload(0) {
        sstore(0x02, 0x0202)
    }
    sstore(0x03, 0x003)
}
// ----
// Block 0:
//   Entries: None
//   sstore: [ 0x0101 0x01 ] => [ ]
//   calldataload: [ 0x00 ] => [ TMP[calldataload, 0] ]
//   ConditionalJump TMP[calldataload, 0]:
//     NonZero: 1
//     Zero: 2
// Block 1:
//   Entries: 0
//   sstore: [ 0x0202 0x02 ] => [ ]
//   Jump: 2
// Block 2:
//   Entries: 0, 1
//   sstore: [ 0x03 0x03 ] => [ ]
//   MainExit
