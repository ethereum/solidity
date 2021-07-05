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
//   Entry Layout: [ ]
//   [ 0x0101 0x01 ] >> sstore
//   [ 0x00 ] >> calldataload
//   Exit Layout: [ TMP[calldataload, 0] ]
//   ConditionalJump TMP[calldataload, 0]:
//     NonZero: 1 (Entry Layout: [ ])
//     Zero: 2 (Entry Layout: [ ])
// Block 1:
//   Entries: 0
//   Entry Layout: [ ]
//   [ 0x0202 0x02 ] >> sstore
//   Exit Layout: [ ]
//   Jump: 2 (Entry Layout: [ ])
// Block 2:
//   Entries: 0, 1
//   Entry Layout: [ ]
//   [ 0x03 0x03 ] >> sstore
//   Exit Layout: [ ]
//   MainExit
