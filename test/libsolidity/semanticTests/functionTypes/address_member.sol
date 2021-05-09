contract C {
    function f() public view returns (address a1, address a2) {
        a1 = this.f.address;
        this.f.address;
        [this.f.address][0];
        a2 = [this.f.address][0];
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 90572315268751552425567948436632610904688605307, 90572315268751552425567948436632610904688605307
