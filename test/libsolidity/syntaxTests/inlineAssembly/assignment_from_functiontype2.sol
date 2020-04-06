contract C {
    function f() public pure {}
    constructor() public {
        assembly {
            let x := f
        }
    }
}
// ----
// DeclarationError: (112-113): Access to functions is not allowed in inline assembly.
