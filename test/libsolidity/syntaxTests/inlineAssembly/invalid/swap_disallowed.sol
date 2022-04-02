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
// DeclarationError 4619: (75-80='swap0'): Function "swap0" not found.
// DeclarationError 4619: (95-100='swap1'): Function "swap1" not found.
// DeclarationError 4619: (115-120='swap2'): Function "swap2" not found.
// DeclarationError 4619: (135-140='swap3'): Function "swap3" not found.
// DeclarationError 4619: (155-160='swap4'): Function "swap4" not found.
// DeclarationError 4619: (175-180='swap5'): Function "swap5" not found.
// DeclarationError 4619: (195-200='swap6'): Function "swap6" not found.
// DeclarationError 4619: (215-220='swap7'): Function "swap7" not found.
// DeclarationError 4619: (235-240='swap8'): Function "swap8" not found.
// DeclarationError 4619: (255-260='swap9'): Function "swap9" not found.
// DeclarationError 4619: (275-281='swap10'): Function "swap10" not found.
// DeclarationError 4619: (296-302='swap11'): Function "swap11" not found.
// DeclarationError 4619: (317-323='swap12'): Function "swap12" not found.
// DeclarationError 4619: (338-344='swap13'): Function "swap13" not found.
// DeclarationError 4619: (359-365='swap14'): Function "swap14" not found.
// DeclarationError 4619: (380-386='swap15'): Function "swap15" not found.
// DeclarationError 4619: (401-407='swap16'): Function "swap16" not found.
// DeclarationError 4619: (422-428='swap32'): Function "swap32" not found.
