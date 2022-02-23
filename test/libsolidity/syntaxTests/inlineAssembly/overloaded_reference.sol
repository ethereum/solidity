contract C {
    function f() pure public {}
    function f(address) pure public {}
    function g() pure public {
        assembly {
            let x := f
        }
    }
}
// ----
// DeclarationError 4718: (155-156): Multiple matching identifiers. Resolving overloaded identifiers is not supported.
