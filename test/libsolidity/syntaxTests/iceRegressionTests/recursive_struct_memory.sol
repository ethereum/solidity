contract Test {
        struct RecursiveStruct {
                address payable d ;
                mapping ( uint => address payable ) c ;
                mapping ( uint => address payable [ ] ) d ;
        }
        function func ( ) private pure {
                RecursiveStruct [ 1 ] memory val ;
                val ;
        }
}
// ----
// DeclarationError: (157-198): Identifier already declared.
// TypeError: (268-300): Type struct Test.RecursiveStruct[1] is only valid in storage.
