contract C {
    function f(string memory s) external pure returns (string memory) {
        return s;
    }
}
// NOTE: The test is here to illustrate the problem with formatting control chars in strings in
// test expectations but unfortunately it can only be triggered manually. It does not test anything
// unless you introduce a difference in expectations to force isoltest to reformat them.
// ====
// compileViaYul: also
// ----
// f(string): 0x20, 16, "\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83" -> 0x20, 16, "\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83" # Input/Output: "😃😃😃😃" #
