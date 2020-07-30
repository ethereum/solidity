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
// DeclarationError 4619: (75-80): Function not found.
// DeclarationError 4619: (95-100): Function not found.
// DeclarationError 4619: (115-120): Function not found.
// DeclarationError 4619: (135-140): Function not found.
// DeclarationError 4619: (155-160): Function not found.
// DeclarationError 4619: (175-180): Function not found.
// DeclarationError 4619: (195-200): Function not found.
// DeclarationError 4619: (215-220): Function not found.
// DeclarationError 4619: (235-240): Function not found.
// DeclarationError 4619: (255-260): Function not found.
// DeclarationError 4619: (275-281): Function not found.
// DeclarationError 4619: (296-302): Function not found.
// DeclarationError 4619: (317-323): Function not found.
// DeclarationError 4619: (338-344): Function not found.
// DeclarationError 4619: (359-365): Function not found.
// DeclarationError 4619: (380-386): Function not found.
// DeclarationError 4619: (401-407): Function not found.
// DeclarationError 4619: (422-428): Function not found.
