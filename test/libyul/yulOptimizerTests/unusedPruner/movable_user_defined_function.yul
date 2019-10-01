{
    function f(x) -> t {
        let b := 7
    }
    function g(x) -> t {
        t := x
    }
    let x := f(g(2))
}
// ====
// step: unusedPruner
// ----
// { }
