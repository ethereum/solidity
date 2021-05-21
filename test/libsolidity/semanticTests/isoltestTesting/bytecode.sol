contract SmokeTest {
    constructor()
    {
    }
}
// ====
// compileViaYul: also
// ----
// constructor()
// bytecode: 0x00112233 -> 0
// bytecode: 0x60806040526 -> 1
// bytecode: 0x806040526 -> 1
// bytecode: 0x6040526 -> 1
// bytecode: 0x5040526 -> 0
// bytecode: "(60).*" -> 1
// bytecode: ".*(40).*" -> 1
// bytecode: ".*40.*6.*" -> 1
// bytecode: ".*40.*600000.*" -> 0
// bytecode: ".*40.*600011.*" -> 0
// bytecode: ".*40.*6000.*" -> 1
