contract C {
    uint public a = 0x4200 >> 8;
}

// ----
// a() -> 0x42
// a():"" -> "66"
