contract C {
    function f() pure public {
        assembly {
            let x := msize()
        }
    }
}
// ====
// optimize-yul: false
// ----
