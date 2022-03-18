
### `ir-no-optimize`
|   project |  bytecode_size | deployment_gas |     method_gas |
|:---------:|---------------:|---------------:|---------------:|
|    bleeps |                |                |                |
|    colony |                |                |                |
| elementfi |                |                |           `0%` |
|       ens |           `!A` |           `!A` |           `!A` |
|     euler | **`+1.43% ❌`** |           `0%` | **`+2.47% ❌`** |
|    gnosis |           `!B` |           `!B` |           `!B` |
|  zeppelin |                |                |                |

### `ir-optimize-evm+yul`
|   project |   bytecode_size |  deployment_gas | method_gas |
|:---------:|----------------:|----------------:|-----------:|
|    bleeps |  **`+0.53% ❌`** |            `0%` |      `-0%` |
|    colony |            `!A` |            `!A` |       `!A` |
| elementfi |                 |                 |            |
|       ens |            `!A` |            `!A` |       `!A` |
|     euler | **`+12.64% ❌`** | **`+11.98% ❌`** |       `0%` |
|    gnosis |            `!B` |            `!B` |       `!B` |
|  zeppelin |                 |                 |            |

### `ir-optimize-evm-only`
|   project | bytecode_size | deployment_gas | method_gas |
|:---------:|--------------:|---------------:|-----------:|
|    bleeps |               |                |            |
|    colony |               |                |            |
| elementfi |          `!B` |           `!B` |       `!B` |
|       ens |          `!A` |           `!A` |       `!A` |
|     euler |          `!V` |           `!V` |       `!V` |
|    gnosis |          `!B` |           `!B` |       `!B` |
|  zeppelin |               |                |            |

### `legacy-no-optimize`
|   project | bytecode_size | deployment_gas | method_gas |
|:---------:|--------------:|---------------:|-----------:|
|    bleeps |               |                |            |
|    colony |          `!B` |           `!B` |       `!B` |
| elementfi |          `!A` |           `!B` |            |
|       ens |          `!A` |           `!A` |       `!A` |
|     euler |          `!V` |           `!V` |       `!V` |
|    gnosis |          `!B` |           `!B` |       `!B` |
|  zeppelin |               |                |            |

### `legacy-optimize-evm+yul`
|   project | bytecode_size | deployment_gas | method_gas |
|:---------:|--------------:|---------------:|-----------:|
|    bleeps |          `0%` |           `0%` |       `0%` |
|    colony |          `0%` |                |            |
| elementfi |          `!A` |           `!B` |            |
|       ens |          `!A` |           `!A` |       `!A` |
|     euler |          `!V` |           `!V` |       `!V` |
|    gnosis |          `!B` |           `!B` |       `!B` |
|  zeppelin |          `0%` |           `0%` |            |

### `legacy-optimize-evm-only`
|   project | bytecode_size | deployment_gas | method_gas |
|:---------:|--------------:|---------------:|-----------:|
|    bleeps |               |                |            |
|    colony |               |                |            |
| elementfi |          `!A` |           `!A` |       `!A` |
|       ens |          `!A` |           `!A` |       `!A` |
|     euler |          `!V` |           `!V` |       `!V` |
|    gnosis |          `!B` |           `!B` |       `!B` |
|  zeppelin |               |                |            |


`!V` = version mismatch
`!B` = no value in the "before" version
`!A` = no value in the "after" version
`!T` = one or both values were not numeric and could not be compared
`-0` = very small negative value rounded to zero
`+0` = very small positive value rounded to zero

