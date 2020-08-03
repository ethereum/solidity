{
    datacopy(0, 1, 2)
    datasize("")
    datasize(0) // This should not be valid.
    dataoffset("")
    dataoffset(0) // This should not be valid.
}
// ----
// TypeError 3517: (37-39): Unknown data object "".
// TypeError 5859: (54-55): Function expects string literal.
// TypeError 3517: (101-103): Unknown data object "".
// TypeError 5859: (120-121): Function expects string literal.
