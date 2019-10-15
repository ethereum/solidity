contract C {
    struct S { uint x; }
    S s;
    function e() pure public {
        assembly { let x := s_offset let y := mul(s_slot, 2) }
    }
}

// ----
