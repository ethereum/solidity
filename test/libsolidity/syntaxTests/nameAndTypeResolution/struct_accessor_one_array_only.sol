contract test {
    struct Data { uint[15] m_array; }
    Data public data;
}
// ----
// TypeError: (58-74): Internal or recursive type is not allowed for public state variables.
