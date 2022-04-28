contract C {
    function f() public {
        assembly {
            datasize(0)
            dataoffset(0)
            datacopy(0, 1, 2)
            setimmutable(0, "x", 1)
            loadimmutable("x")
            linkersymbol("x")
            memoryguard(0)
            verbatim_1i_1o(hex"600202", 0)

            pop(msize())
            pop(pc())
        }
    }
}
// ----
// SyntaxError 6553: (47-362): The msize instruction cannot be used when the Yul optimizer is activated because it can change its semantics. Either disable the Yul optimizer or do not use the instruction.
// DeclarationError 4619: (70-78): Function "datasize" not found.
// DeclarationError 4619: (94-104): Function "dataoffset" not found.
// DeclarationError 4619: (120-128): Function "datacopy" not found.
// DeclarationError 4619: (150-162): Function "setimmutable" not found.
// DeclarationError 4619: (186-199): Function "loadimmutable" not found.
// DeclarationError 4619: (217-229): Function "linkersymbol" not found.
// DeclarationError 4619: (247-258): Function "memoryguard" not found.
// DeclarationError 4619: (274-288): Function "verbatim_1i_1o" not found.
// SyntaxError 2450: (347-349): PC instruction is a low-level EVM feature. Because of that PC is disallowed in strict assembly.
