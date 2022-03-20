contract C {
    function f() public pure returns (uint r) {
        assembly { function f() -> x { x := 1 } r := f() }
    }
    function g() public pure returns (uint r) {
        assembly { function f() -> x { x := 2 } r := f() }
    }
}
// ----
// DeclarationError 3859: (80-108): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 3859: (193-221): This declaration shadows a declaration outside the inline assembly block.
