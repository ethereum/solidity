contract C
{
    function f() public pure {
        blobhash;
    }
}
// ====
// EVMVersion: <=shanghai
// ----
// DeclarationError 7576: (52-60): Undeclared identifier. Did you mean "blockhash"?
