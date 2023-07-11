contract C {
    struct S { uint x; }
    S s;
    function e() pure public {
        assembly { let x := s.offset let y := mul(s.slot, 2) }
    }
}
// ----
