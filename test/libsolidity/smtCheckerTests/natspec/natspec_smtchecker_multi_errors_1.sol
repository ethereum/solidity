contract C {
    /// @custom:smtchecker b
    /// @custom:smtchecker
    /// @custom:smtchecker a b c
    function f() internal {}
}

contract D is C {}
// ----
// Warning 3130: (106-130): Unknown option for "custom:smtchecker": "b"
