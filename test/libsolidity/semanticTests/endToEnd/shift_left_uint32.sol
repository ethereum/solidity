contract C {
    function f(uint32 a, uint32 b) public returns(uint) {
        return a << b;
    }
}

// ----
// f(uint32,uint32): 0x4266), 0) -> 0x4266
// f(uint32,uint32):"16998, 0" -> "16998"
// f(uint32,uint32): 0x4266), 8) -> 0x426600
// f(uint32,uint32):"16998, 8" -> "4351488"
// f(uint32,uint32): 0x4266), 16) -> 0x42660000
// f(uint32,uint32):"16998, 16" -> "1113980928"
// f(uint32,uint32): 0x4266), 17) -> 0x84cc0000
// f(uint32,uint32):"16998, 17" -> "2227961856"
// f(uint32,uint32): 0x4266), 32) -> 0
// f(uint32,uint32):"16998, 32" -> "0"
