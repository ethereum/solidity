contract C {
    function f0() public { (()) = 2; }

    function f1() public pure { (()) = (); }

    //#8711
    function f2() internal pure returns (uint, uint) { return () = f2(); }

    //#8277
    function f3()public{return()=();}

    //#8277
    function f4 ( bytes32 hash , uint8 v , bytes32 r , bytes32 s , uint blockExpired , bytes32 salt ) public returns ( address ) {
        require ( ( ( ) ) |= keccak256 ( abi . encodePacked ( blockExpired , salt ) ) ) ;
        return ecrecover ( hash , v , r , s ) ;
    }
}
// ----
// TypeError: (41-43): Empty tuple on the left hand side.
// TypeError: (47-48): Type int_const 2 is not implicitly convertible to expected type tuple().
// TypeError: (86-88): Empty tuple on the left hand side.
// TypeError: (173-175): Empty tuple on the left hand side.
// TypeError: (178-182): Type tuple(uint256,uint256) is not implicitly convertible to expected type tuple().
// TypeError: (166-182): Different number of arguments in return statement than in returns declaration.
// TypeError: (229-231): Empty tuple on the left hand side.
// TypeError: (401-404): Empty tuple on the left hand side.
// TypeError: (399-466): Compound assignment is not allowed for tuple types.
// TypeError: (410-466): Type bytes32 is not implicitly convertible to expected type tuple().
// TypeError: (389-396): No matching declaration found after argument-dependent lookup.
