{
    let x := 0x01
    let y := 0x02
    sstore(0x01, x)
    for { sstore(0x02, 0x0202) } lt(x, 0x0303) { x := add(x,0x0404) } {
        sstore(0x05, 0x0505)
        y := sload(x)
    }
    sstore(0x06, 0x0506)
}
// ----
// Block 0:
//   Entries: None
//   Entry Layout: [ ]
//   [ 0x0404 0x01 ] >> Assignment(x)
//   [ 0x0404 x 0x02 ] >> Assignment(y)
//   [ 0x0404 x x 0x01 ] >> sstore
//   [ 0x0404 x 0x0202 0x02 ] >> sstore
//   Exit Layout: [ 0x0404 x ]
//   Jump: 1 (Entry Layout: [ 0x0404 0x0404 x ])
// Block 1:
//   Entries: 0, 2
//   Entry Layout: [ 0x0404 0x0404 x ]
//   [ 0x0404 0x0404 x 0x0303 x ] >> lt
//   Exit Layout: [ 0x0404 0x0404 x TMP[lt, 0] ]
//   ConditionalJump TMP[lt, 0]:
//     NonZero: 3 (Entry Layout: [ 0x0404 0x0404 x ])
//     Zero: 4 (Entry Layout: [ JUNK JUNK JUNK ])
// Block 2:
//   Entries: 3
//   Entry Layout: [ 0x0404 0x0404 x ]
//   [ 0x0404 0x0404 x ] >> add
//   [ 0x0404 TMP[add, 0] ] >> Assignment(x)
//   Exit Layout: [ 0x0404 x ]
//   Jump (backwards): 1 (Entry Layout: [ 0x0404 0x0404 x ])
// Block 3:
//   Entries: 1
//   Entry Layout: [ 0x0404 0x0404 x ]
//   [ 0x0404 0x0404 x 0x0505 0x05 ] >> sstore
//   [ 0x0404 0x0404 x x ] >> sload
//   [ 0x0404 0x0404 x TMP[sload, 0] ] >> Assignment(y)
//   Exit Layout: [ 0x0404 0x0404 x ]
//   Jump: 2 (Entry Layout: [ 0x0404 0x0404 x ])
// Block 4:
//   Entries: 1
//   Entry Layout: [ JUNK JUNK JUNK ]
//   [ 0x0506 0x06 ] >> sstore
//   Exit Layout: [ ]
//   MainExit
