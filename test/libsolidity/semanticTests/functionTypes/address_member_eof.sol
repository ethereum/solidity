contract C {
    function f() public view returns (address a1, address a2) {
        a1 = this.f.address;
        this.f.address;
        [this.f.address][0];
        a2 = [this.f.address][0];
    }
}
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> 0xab639b56c881a5b607f4a706bcf1d7d383b83703, 0xab639b56c881a5b607f4a706bcf1d7d383b83703
