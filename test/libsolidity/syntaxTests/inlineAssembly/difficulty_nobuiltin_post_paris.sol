contract C {
    function f() public {
        assembly {
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: >=paris
// ----
// DeclarationError 4619: (74-84): Function "difficulty" not found.
// TypeError 3950: (74-86): Expected expression to evaluate to one value, but got 0 values instead.
