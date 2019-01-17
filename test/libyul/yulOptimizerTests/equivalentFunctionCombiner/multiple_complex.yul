{
  pop(f(1,2,3))
  pop(g(4,5,6))
  pop(h(7,8,9))
  function f(f1, f2, f3) -> rf
  {
    switch f1
        case 0 {
            if f2
            {
                rf := f3
            }
            if not(f2)
            {
                rf := f1
            }
        }
        default {
            rf := 3
        }
  }
  function g(g1, g2, g3) -> rg
  {
    switch g1
        case 0 {
            if g2
            {
                rg := g3
            }
            if not(g2)
            {
                rg := g1
            }
        }
        default {
            rg := 3
        }
  }
  function h(h1, h2, h3) -> rh
  {
    switch h1
        case 1 {
            if h2
            {
                rh := h3
            }
            if not(h2)
            {
                rh := h1
            }
        }
        default {
            rh := 3
        }
  }
}
// ----
// equivalentFunctionCombiner
// {
//     pop(f(1, 2, 3))
//     pop(f(4, 5, 6))
//     pop(h(7, 8, 9))
//     function f(f1, f2, f3) -> rf
//     {
//         switch f1
//         case 0 {
//             if f2
//             {
//                 rf := f3
//             }
//             if not(f2)
//             {
//                 rf := f1
//             }
//         }
//         default {
//             rf := 3
//         }
//     }
//     function g(g1, g2, g3) -> rg
//     {
//         switch g1
//         case 0 {
//             if g2
//             {
//                 rg := g3
//             }
//             if not(g2)
//             {
//                 rg := g1
//             }
//         }
//         default {
//             rg := 3
//         }
//     }
//     function h(h1, h2, h3) -> rh
//     {
//         switch h1
//         case 1 {
//             if h2
//             {
//                 rh := h3
//             }
//             if not(h2)
//             {
//                 rh := h1
//             }
//         }
//         default {
//             rh := 3
//         }
//     }
// }
