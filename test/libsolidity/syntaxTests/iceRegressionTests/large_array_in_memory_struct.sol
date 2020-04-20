contract C {
        struct X { bytes31 [ 3 ] x1 ;
                uint x2 ;
        }
        struct S { uint256 [ ] [ 0.425781 ether ] s1 ;
                uint [ 2 ** 0xFF ] [ 2 ** 0x42 ] s2 ;
                X s3 ;
                mapping ( uint => address payable ) c ;
                uint [ 9 hours ** 16 ] d ;
                string s ;
        }
        function f ( ) public { function ( function ( bytes9 , uint ) external pure returns ( uint ) , uint ) external pure returns ( uint ) [ 3 ] memory s2 ;
                S memory s ;
        }
}
// ----
// TypeError: (530-540): Type too large for memory.
