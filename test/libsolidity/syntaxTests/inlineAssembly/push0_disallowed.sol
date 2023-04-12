// Ok
contract push0 {}

contract A {
    // Ok, warning about shadowing
    function push0() external {}
}

contract C {
    function f() external {
        assembly {
            // Not okay
            push0()
        }
    }
}
// ----
// Warning 2519: (77-105): This declaration shadows an existing declaration.
// DeclarationError 4619: (205-210): Function "push0" not found.
