contract C {
    function f() public {
        assembly {
            pop(prevrandao())
        }
    }
}
// ====
// EVMVersion: <paris
// ----
// DeclarationError 4619: (74-84): Function "prevrandao" not found.
// TypeError 3950: (74-86): Expected expression to evaluate to one value, but got 0 values instead.
