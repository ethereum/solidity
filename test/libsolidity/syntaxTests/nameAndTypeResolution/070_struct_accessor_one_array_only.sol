contract test {
    struct Data { uint[15] m_array; }
    Data public data;
}
// ----
// TypeError 5359: (58-74): The struct has all its members omitted, therefore the getter cannot return any values.
