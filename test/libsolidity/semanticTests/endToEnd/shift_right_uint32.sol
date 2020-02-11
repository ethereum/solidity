contract C {
    function f(uint32 a, uint32 b) public returns(uint) {
        return a >> b;
    }
}

// ----
// f(uint32,uint32): 0x4266), 0) -> 0x4266
// f(uint32,uint32):"16998, 0" -> "16998"
// f(uint32,uint32): 0x4266), 8) -> 0x42
// f(uint32,uint32):"16998, 8" -> "66"
// f(uint32,uint32): 0x4266), 16) -> 0
// f(uint32,uint32):"16998, 16" -> "0"
// f(uint32,uint32): 0x4266), 17) -> 0
// f(uint32,uint32):"16998, 17" -> "0"
