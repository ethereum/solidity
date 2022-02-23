contract C {
    function f() pure public {
        assembly {
            let x := msize()
        }
    }
}
// ====
// optimize-yul: true
// ----
// SyntaxError 6553: (52-101): The msize instruction cannot be used when the Yul optimizer is activated because it can change its semantics. Either disable the Yul optimizer or do not use the instruction.
