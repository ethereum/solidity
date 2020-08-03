{
    datacopy(0, 1, 2)
    datasize("")
    datasize(0) // This should not be valid.
    dataoffset("")
    dataoffset(0) // This should not be valid.
}
// ----
// TypeError 3517: (28-36): Unknown data object "".
// TypeError 9427: (45-53): Data object name is expected to be a string.
// TypeError 3517: (90-100): Unknown data object "".
// TypeError 9427: (109-119): Data object name is expected to be a string.