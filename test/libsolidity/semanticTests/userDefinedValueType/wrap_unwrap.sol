type MyAddress is address;
contract C {
    function f() pure public {
        MyAddress.wrap;
        MyAddress.unwrap;
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
