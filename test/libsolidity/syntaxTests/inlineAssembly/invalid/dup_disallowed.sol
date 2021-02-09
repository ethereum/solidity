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
// DeclarationError 4619: (75-79): Function "dup0" not found.
// DeclarationError 4619: (94-98): Function "dup1" not found.
// DeclarationError 4619: (113-117): Function "dup2" not found.
// DeclarationError 4619: (132-136): Function "dup3" not found.
// DeclarationError 4619: (151-155): Function "dup4" not found.
// DeclarationError 4619: (170-174): Function "dup5" not found.
// DeclarationError 4619: (189-193): Function "dup6" not found.
// DeclarationError 4619: (208-212): Function "dup7" not found.
// DeclarationError 4619: (227-231): Function "dup8" not found.
// DeclarationError 4619: (246-250): Function "dup9" not found.
// DeclarationError 4619: (265-270): Function "dup10" not found.
// DeclarationError 4619: (285-290): Function "dup11" not found.
// DeclarationError 4619: (305-310): Function "dup12" not found.
// DeclarationError 4619: (325-330): Function "dup13" not found.
// DeclarationError 4619: (345-350): Function "dup14" not found.
// DeclarationError 4619: (365-370): Function "dup15" not found.
// DeclarationError 4619: (385-390): Function "dup16" not found.
// DeclarationError 4619: (405-410): Function "dup32" not found.
