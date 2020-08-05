## yul-phaser
`yul-phaser` is an internal tool for finding good sequences of [optimisation steps](/libyul/optimiser/README.md) for Yul optimiser.

### How it works
The space of possible solutions to this problem (usually referred to as _phase-ordering problem_) is extremely large and there may even be no single sequence that produces optimal results for all possible programs.

The tool uses genetic algorithms to find sequences that result in better programs than others and to iteratively refine them.
The input is a set of one or more [Yul](/docs/yul.rst) programs and each sequence is applied to all of these programs.
Optimised programs are given numeric scores according to the selected metric.

Optimisation step sequences are presented in an abbreviated form - as strings of letters where each character represents one step.
There's a [table listing available abbreviations in the optimiser docs](/docs/yul.rst#optimization-step-sequence).

### How to use it
The application has sensible defaults for most parameters.
An invocation can be as simple as:

``` bash
tools/yul-phaser ../test/libyul/yulOptimizerTests/fullSuite/*.yul \
    --random-population 100
```

This assumes that you have a working copy of the Solidity repository and you're in the build directory within that working copy.

Run `yul-phaser --help` for a full list of available options.

#### Restarting from a previous state
`yul-phaser` can save the list of sequences found after each round:

``` bash
tools/yul-phaser *.yul        \
    --random-population   100 \
    --population-autosave /tmp/population.txt
```

If you stop the application, you can later use the file to continue the search from the point you left off:

``` bash
tools/yul-phaser *.yul                         \
    --population-from-file /tmp/population.txt \
    --population-autosave  /tmp/population.txt
```

#### Analysing a sequence
Apart from running the genetic algorithm, `yul-phaser` can also provide useful information about a particular sequence.

For example, to see the value of a particular metric for a given sequence and program run:
``` bash
tools/yul-phaser *.yul            \
    --show-initial-population     \
    --rounds            0         \
    --metric            code-size \
    --metric-aggregator sum       \
    --population        <your sequence>
```

You can also easily see program code after being optimised using that sequence:
``` bash
tools/yul-phaser *.yul                    \
    --rounds     0                        \
    --mode       print-optimised-programs \
    --population <your sequence>
```

#### Using output from Solidity compiler
`yul-phaser` can process the intermediate representation produced by `solc`:

``` bash
solc/solc <sol file> --ir --output-dir <output directory>
```

After running this command you'll find one or more .yul files in the output directory.
These files contain whole Yul objects rather than just raw Yul programs but `yul-phaser` is prepared to handle them too.

#### Using optimisation step sequences with the compiler
You can tell Yul optimiser to use a specific sequence for your code by passing `--yul-optimizations` option to `solc`:

``` bash
solc/solc <sol file> --optimize --ir-optimized --yul-optimizations <sequence>
```

### How to choose good parameters
Choosing good parameters for a genetic algorithm is not a trivial task but phaser's defaults are generally enough to find a sequence that gives results comparable or better than one hand-crafted by an experienced developer for a given set of programs.
The difficult part is providing a fairly representative set of input files.
If the files you give don't need certain optimisations the tool will find sequences that don't use these optimisations and perform badly for programs that could benefit from them.
If all the provided files greatly benefit from a specific optimisation, the sequence may not work well for programs that do not.

We have conducted [a set of rough experiments](https://github.com/ethereum/solidity/issues/7806#issuecomment-598644491) to evaluate some combinations of parameter values.
The conclusions were used to adjust the defaults but you might still benefit from some general observations:

1. The algorithm that performed the best was `GEWEP`.
2. Using longer sequences in the initial population yields better results. The algorithm is good at removing superfluous steps.
3. Preserving the top sequences from previous rounds improves results. Elite should contain at least a few individuals, especially when using the `classic` algorithm.
4. Don't set mutation/deletion/addition chance too high. It makes results worse because it destroys the good patterns preserved by crossover. Values around 1-5% seem to work best.
5. Keep the algorithm running for 1000 rounds or more. It usually finds good sequences faster than that but it can shorten them significantly if you let it run longer. This is especially important when starting with long sequences.
