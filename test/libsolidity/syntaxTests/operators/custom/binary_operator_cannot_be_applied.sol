type Int is int256;

function f() pure {
    Int a = Int.wrap(0);
    a + a;
    a >>> a;
}

// ----
// TypeError 2271: (70-75): Binary operator + not compatible with types Int and Int. No matching user-defined operator found.
// TypeError 2271: (81-88): Binary operator >>> not compatible with types Int and Int.
