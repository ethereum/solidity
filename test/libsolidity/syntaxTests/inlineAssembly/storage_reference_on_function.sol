contract C {
    function f() pure public {
        assembly {
            let x := f.slot
        }
    }
}
// ----
// TypeError 7944: (84-90): The suffixes ".offset", ".slot" and ".length" can only be used with variables.
