contract C {
    int public a = -0x42 << 8;
}

// ----
// a() -> -16896
