{
    let x := 0x0101
    let y := 0x0202
    let z := 0x0303
    switch sload(x)
    case 0 {
        x := 0x42
    }
    case 1 {
        y := 0x42
    }
    default {
        sstore(z, z)
    }

    sstore(0x0404, y)
}
// ----
// Block 0:
//   Entries: None
//   Entry Layout: [ ]
//   [ 0x0101 ] >> Assignment(x)
//   [ x 0x0202 ] >> Assignment(y)
//   [ y x 0x0303 ] >> Assignment(z)
//   [ y z x ] >> sload
//   [ y z TMP[sload, 0] ] >> Assignment(GHOST[0])
//   [ y z GHOST[0] GHOST[0] 0x00 ] >> eq
//   Exit Layout: [ y z GHOST[0] TMP[eq, 0] ]
//   ConditionalJump TMP[eq, 0]:
//     NonZero: 1 (Entry Layout: [ y JUNK JUNK ])
//     Zero: 2 (Entry Layout: [ y z GHOST[0] ])
// Block 1:
//   Entries: 0
//   Entry Layout: [ y JUNK JUNK ]
//   [ y 0x42 ] >> Assignment(x)
//   Exit Layout: [ y ]
//   Jump: 3 (Entry Layout: [ y ])
// Block 2:
//   Entries: 0
//   Entry Layout: [ y z GHOST[0] ]
//   [ y z GHOST[0] 0x01 ] >> eq
//   Exit Layout: [ y z TMP[eq, 0] ]
//   ConditionalJump TMP[eq, 0]:
//     NonZero: 4 (Entry Layout: [ JUNK JUNK ])
//     Zero: 5 (Entry Layout: [ y z ])
// Block 3:
//   Entries: 1, 4, 5
//   Entry Layout: [ y ]
//   [ y 0x0404 ] >> sstore
//   Exit Layout: [ ]
//   MainExit
// Block 4:
//   Entries: 2
//   Entry Layout: [ JUNK JUNK ]
//   [ 0x42 ] >> Assignment(y)
//   Exit Layout: [ y ]
//   Jump: 3 (Entry Layout: [ y ])
// Block 5:
//   Entries: 2
//   Entry Layout: [ y z ]
//   [ y z z ] >> sstore
//   Exit Layout: [ y ]
//   Jump: 3 (Entry Layout: [ y ])
