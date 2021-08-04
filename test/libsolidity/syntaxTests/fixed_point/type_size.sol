contract A
{
    function s128x18() public pure returns (ufixed16x16 x) {
        x = 0;
    }
}
// ----
// TypeError 5108: (57-70): Invalid fixed point type ufixed16x16: 10^16 does not fit in 2^16 bits.
