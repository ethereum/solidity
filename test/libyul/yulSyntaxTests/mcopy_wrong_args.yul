{
    mcopy(0x100, 0x200, 0x300, 0x400)
    mcopy(0x100, 0x200)
    mcopy(0x100)
    mcopy()
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 7000: (6-11): Function "mcopy" expects 3 arguments but got 4.
// TypeError 7000: (44-49): Function "mcopy" expects 3 arguments but got 2.
// TypeError 7000: (68-73): Function "mcopy" expects 3 arguments but got 1.
// TypeError 7000: (85-90): Function "mcopy" expects 3 arguments but got 0.
