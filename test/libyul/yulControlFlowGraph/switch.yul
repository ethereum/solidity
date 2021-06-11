{
    sstore(0, 0)
    switch sload(0)
    case 0 {
        sstore(0x01, 0x0101)
    }
    case 1 {
        sstore(0x02, 0x0101)
    }
    default {
        sstore(0x03, 0x0101)
    }
    sstore(0x04, 0x0101)
}
// ----
// Block 0:
//   Entries: None
//   sstore: [ 0x00 0x00 ] => [ ]
//   sload: [ 0x00 ] => [ TMP[sload, 0] ]
//   Assignment(GHOST[0]): [ TMP[sload, 0] ] => [ GHOST[0] ]
//   eq: [ GHOST[0] 0x00 ] => [ TMP[eq, 0] ]
//   ConditionalJump TMP[eq, 0]:
//     NonZero: 1
//     Zero: 2
// Block 1:
//   Entries: 0
//   sstore: [ 0x0101 0x01 ] => [ ]
//   Jump: 3
// Block 2:
//   Entries: 0
//   eq: [ GHOST[0] 0x01 ] => [ TMP[eq, 0] ]
//   ConditionalJump TMP[eq, 0]:
//     NonZero: 4
//     Zero: 5
// Block 3:
//   Entries: 1, 4, 5
//   sstore: [ 0x0101 0x04 ] => [ ]
//   MainExit
// Block 4:
//   Entries: 2
//   sstore: [ 0x0101 0x02 ] => [ ]
//   Jump: 3
// Block 5:
//   Entries: 2
//   sstore: [ 0x0101 0x03 ] => [ ]
//   Jump: 3
