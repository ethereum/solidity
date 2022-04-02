{
    let verbatim := 2
    let verbatim_abc := 3
    function verbatim_fun() {}
}
// ----
// DeclarationError 5017: (10-18='verbatim'): The identifier "verbatim" is reserved and can not be used.
// DeclarationError 5017: (32-44='verbatim_abc'): The identifier "verbatim_abc" is reserved and can not be used.
// DeclarationError 5017: (54-80='function verbatim_fun() {}'): The identifier "verbatim_fun" is reserved and can not be used.
