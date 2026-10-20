// SPDX-License-Identifier: GPL-3.0
// reproducing https://github.com/argotorg/solidity/issues/16155

pragma solidity >=0.8.19 <0.10.0;

contract PlaceholderContract {
    function tZhBDeXU4NnUR6(bool assert_in1) internal virtual returns (int128) {
        return ((
            (assert_in1 && false)
                ? (int128(638) - int128(932))
                : (assert_in1 ? int128(923) : int128(392))
        ) / ((int128(517) * int128(573)) / (int128(273) % int128(605))));
    }

    function QtdPeAwoi7LD(
        bool assert_in1,
        bool assert_in2
    ) internal pure returns (int128) {
        return ((
            (assert_in2 || assert_in1)
                ? (int128(478) % int128(983))
                : (int128(298) + int128(316))
        ) +
            (
                (assert_in1 && assert_in1)
                    ? (int128(71) / int128(885))
                    : (-int128(596))
            ));
    }

    function fmMG$apfQ14W86hu_3M(
        bool assert_out2
    ) internal virtual returns (bool) {
        return assert_out2;
    }

    function check_entrypoint(
        bool /*assert_in0*/,
        bool assert_in1,
        bool assert_in2,
        bool /*assert_in3*/
    ) public {
        unchecked {
            bool assert_out1 = (tZhBDeXU4NnUR6(assert_in1) <
                QtdPeAwoi7LD(assert_in1, assert_in2));
            bool assert_out2 = ((((
                false
                    ? (int128(638) - int128(932))
                    : ((((
                        (!(!assert_in1))
                            ? int128(923)
                            : ((int128(392) + (int128(0) & int128(82))) &
                                ((int128(0) + int128(392)) & int128(392)))
                    ) + int128(0)) - (int128(77) & int128(0))) /
                        (int128(1) | int128(0)))
            ) /
                ((int128(573) * int128(517)) /
                    (-(-(-(int128(0) + (-(int128(273) % int128(605))))))))) <
                ((
                    (true && (assert_in2 || assert_in1))
                        ? (((int128(478) % int128(983)) - int128(76)) +
                            int128(76))
                        : (int128(298) + int128(316))
                ) +
                    (
                        assert_in1
                            ? (int128(73) +
                                ((((int128(71) - int128(94)) + int128(94)) /
                                    int128(885)) - int128(73)))
                            : (-int128(596))
                    ))) ||
                ((((int128(0) |
                    (
                        false
                            ? (int128(638) - int128(932))
                            : ((
                                (!(!(!(!assert_in1))))
                                    ? int128(923)
                                    : (int128(392) &
                                        ((int128(82) & int128(0)) +
                                            int128(392)))
                            ) | int128(0))
                    )) /
                    ((int128(573) * int128(517)) /
                        ((-(-(-(int128(0) - (int128(273) % int128(605)))))) +
                            int128(0)))) | int128(0)) <
                    ((
                        (((assert_in2 || assert_in1) && true) && true)
                            ? (int128(478) % int128(983))
                            : (int128(298) + int128(316))
                    ) +
                        (
                            ((true && assert_in1) && assert_in1)
                                ? (((int128(71) - int128(94)) + int128(94)) /
                                    int128(885))
                                : (-int128(596))
                        ))));
            assert((assert_out1 == fmMG$apfQ14W86hu_3M(assert_out2)));
        }
    }
}
