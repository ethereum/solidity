contract C {
    uint8 constant LEN = 255;
    uint16 constant MORE = 32767;
    uint[LEN] array1;
    uint[LEN + MORE] array2;
    uint[LEN * MORE] array3; // Error, outside uint16 range
}
// ----
// TypeError 2643: (137-147): Arithmetic error when computing constant value.
