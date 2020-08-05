contract C {
    function f() pure public {
        assembly {
            let x := f.slot
        }
    }
}
// ----
// TypeError 7944: (84-90): The suffixes .offset and .slot can only be used on storage variables.
