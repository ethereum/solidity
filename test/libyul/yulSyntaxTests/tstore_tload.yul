{
    function f() {
        let x := tload(0)
        let y := add(x, 1)
        tstore(0, y)
    }
}
// ====
// EVMVersion: >=cancun
// ----
