{
    let name := "long___name___that___definitely___exceeds___the___thirty___two___byte___limit"
}
// ====
// dialect: evm
// ----
// TypeError 3069: (18-97): String literal too long (77 > 32)
