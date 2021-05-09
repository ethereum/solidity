contract C {
    function f() pure public {
        assembly {
            swap0()
            swap1()
            swap2()
            swap3()
            swap4()
            swap5()
            swap6()
            swap7()
            swap8()
            swap9()
            swap10()
            swap11()
            swap12()
            swap13()
            swap14()
            swap15()
            swap16()
            swap32()
        }
    }
}
// ----
// DeclarationError 4619: (75-80): Function "swap0" not found.
// DeclarationError 4619: (95-100): Function "swap1" not found.
// DeclarationError 4619: (115-120): Function "swap2" not found.
// DeclarationError 4619: (135-140): Function "swap3" not found.
// DeclarationError 4619: (155-160): Function "swap4" not found.
// DeclarationError 4619: (175-180): Function "swap5" not found.
// DeclarationError 4619: (195-200): Function "swap6" not found.
// DeclarationError 4619: (215-220): Function "swap7" not found.
// DeclarationError 4619: (235-240): Function "swap8" not found.
// DeclarationError 4619: (255-260): Function "swap9" not found.
// DeclarationError 4619: (275-281): Function "swap10" not found.
// DeclarationError 4619: (296-302): Function "swap11" not found.
// DeclarationError 4619: (317-323): Function "swap12" not found.
// DeclarationError 4619: (338-344): Function "swap13" not found.
// DeclarationError 4619: (359-365): Function "swap14" not found.
// DeclarationError 4619: (380-386): Function "swap15" not found.
// DeclarationError 4619: (401-407): Function "swap16" not found.
// DeclarationError 4619: (422-428): Function "swap32" not found.
