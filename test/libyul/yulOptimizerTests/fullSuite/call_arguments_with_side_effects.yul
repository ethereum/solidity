{
    function fun_revert() -> ret { revert(0, 0) }
    function fun_return() -> ret { return(0, 0) }
    function empty(a, b) {}

    // Evaluation order in Yul is always right to left so optimized code should reach the revert first.
    empty(fun_return(), fun_revert())
}
// ----
// step: fullSuite
//
// { { revert(0, 0) } }
