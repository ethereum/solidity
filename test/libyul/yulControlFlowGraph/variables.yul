{
    let x := calldataload(0)
    let y := calldataload(2)

    x := calldataload(3)
    y := calldataload(4)

    sstore(x,y)
}
// ----
// Block 0:
//   Entries: None
//   calldataload: [ 0x00 ] => [ TMP[calldataload, 0] ]
//   Assignment(x): [ TMP[calldataload, 0] ] => [ x ]
//   calldataload: [ 0x02 ] => [ TMP[calldataload, 0] ]
//   Assignment(y): [ TMP[calldataload, 0] ] => [ y ]
//   calldataload: [ 0x03 ] => [ TMP[calldataload, 0] ]
//   Assignment(x): [ TMP[calldataload, 0] ] => [ x ]
//   calldataload: [ 0x04 ] => [ TMP[calldataload, 0] ]
//   Assignment(y): [ TMP[calldataload, 0] ] => [ y ]
//   sstore: [ y x ] => [ ]
//   MainExit
