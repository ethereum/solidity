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
// Warning: The Yul optimiser is still experimental. Do not use it in production unless correctness of generated code is verified with extensive tests.
// SyntaxError: (52-101): The msize instruction cannot be used when the Yul optimizer is activated because it can change its semantics. Either disable the Yul optimizer or do not use the instruction.
