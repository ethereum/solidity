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
// DeclarationError 4619: (75-79): Function not found.
// DeclarationError 4619: (94-98): Function not found.
// DeclarationError 4619: (113-117): Function not found.
// DeclarationError 4619: (132-136): Function not found.
// DeclarationError 4619: (151-155): Function not found.
// DeclarationError 4619: (170-174): Function not found.
// DeclarationError 4619: (189-193): Function not found.
// DeclarationError 4619: (208-212): Function not found.
// DeclarationError 4619: (227-231): Function not found.
// DeclarationError 4619: (246-250): Function not found.
// DeclarationError 4619: (265-270): Function not found.
// DeclarationError 4619: (285-290): Function not found.
// DeclarationError 4619: (305-310): Function not found.
// DeclarationError 4619: (325-330): Function not found.
// DeclarationError 4619: (345-350): Function not found.
// DeclarationError 4619: (365-370): Function not found.
// DeclarationError 4619: (385-390): Function not found.
// DeclarationError 4619: (405-410): Function not found.
