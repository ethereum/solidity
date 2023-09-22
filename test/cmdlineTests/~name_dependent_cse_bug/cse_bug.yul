object "C" {
    code {}

    object "C_deployed" {
        code {
            main(0, 0)

            function main(a, b) {
                for {} 1 {}
                {
                    if iszero(a) { break }

                    let mid := avg(a, a)
                    switch a
                    case 0 {
                        a := mid
                    }
                    default {
                        sstore(0, mid)
                    }
                }
            }

            function avg(x, y) -> var {
                let __placeholder__ := add(x, y)
                var := add(__placeholder__, __placeholder__)
            }
        }
    }
}
