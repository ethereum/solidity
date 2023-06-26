error MyCustomError(uint, bool);

contract C {
    function f() pure public {
        true ? MyCustomError : MyCustomError;
    }
}

// ----
// TypeError 9717: (93-106): Invalid mobile type in true expression.
// TypeError 3703: (109-122): Invalid mobile type in false expression.
