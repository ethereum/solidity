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
//   Entry Layout: [ ]
//   [ 0x00 ] >> calldataload
//   [ TMP[calldataload, 0] ] >> Assignment(x)
//   [ 0x02 ] >> calldataload
//   [ TMP[calldataload, 0] ] >> Assignment(y)
//   [ 0x03 ] >> calldataload
//   [ TMP[calldataload, 0] ] >> Assignment(x)
//   [ x 0x04 ] >> calldataload
//   [ x TMP[calldataload, 0] ] >> Assignment(y)
//   [ y x ] >> sstore
//   Exit Layout: [ ]
//   MainExit
