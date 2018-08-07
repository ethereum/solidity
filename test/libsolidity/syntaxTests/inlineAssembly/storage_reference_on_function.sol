contract C {
    function f() pure public {
        assembly {
            let x := f_slot
        }
    }
}
// ----
// TypeError: (84-90): The suffixes _offset and _slot can only be used on storage variables.
