contract C {
    function f() pure public {
        assembly {
            dup0()
            dup1()
            dup2()
            dup3()
            dup4()
            dup5()
            dup6()
            dup7()
            dup8()
            dup9()
            dup10()
            dup11()
            dup12()
            dup13()
            dup14()
            dup15()
            dup16()
            dup32()
        }
    }
}
// ----
// DeclarationError 4619: (75-79='dup0'): Function "dup0" not found.
// DeclarationError 4619: (94-98='dup1'): Function "dup1" not found.
// DeclarationError 4619: (113-117='dup2'): Function "dup2" not found.
// DeclarationError 4619: (132-136='dup3'): Function "dup3" not found.
// DeclarationError 4619: (151-155='dup4'): Function "dup4" not found.
// DeclarationError 4619: (170-174='dup5'): Function "dup5" not found.
// DeclarationError 4619: (189-193='dup6'): Function "dup6" not found.
// DeclarationError 4619: (208-212='dup7'): Function "dup7" not found.
// DeclarationError 4619: (227-231='dup8'): Function "dup8" not found.
// DeclarationError 4619: (246-250='dup9'): Function "dup9" not found.
// DeclarationError 4619: (265-270='dup10'): Function "dup10" not found.
// DeclarationError 4619: (285-290='dup11'): Function "dup11" not found.
// DeclarationError 4619: (305-310='dup12'): Function "dup12" not found.
// DeclarationError 4619: (325-330='dup13'): Function "dup13" not found.
// DeclarationError 4619: (345-350='dup14'): Function "dup14" not found.
// DeclarationError 4619: (365-370='dup15'): Function "dup15" not found.
// DeclarationError 4619: (385-390='dup16'): Function "dup16" not found.
// DeclarationError 4619: (405-410='dup32'): Function "dup32" not found.
