{
    let library_name := "contract/library.sol:L"
    let addr := linkersymbol(library_name)
}
// ====
// dialect: evm
// ----
// TypeError 9114: (67-79): Function expects direct literals as arguments.
