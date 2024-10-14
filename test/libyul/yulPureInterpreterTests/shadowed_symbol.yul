{
    function f()
    {
        // Variable declaration does not shadow namesake function declaration
        // because latter not visible here.
        let shadow_id
    }
    {
        // Function named `shadow_id` is in scope now.
        f()
        function shadow_id() {}
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
//   [CALL] f()
//   │ [RETURN]
