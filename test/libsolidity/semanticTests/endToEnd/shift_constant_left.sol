contract C {
    uint public a = 0x42 << 8;
}

// ----
// a() -> 0x4200
// a():"" -> "16896"
