## Solidity external tests
This directory contains scripts for compiling some of the popular open-source projects using the
current version of the compiler and running their test suites.

Since projects often do not use the latest compiler, we keep a fork of each of these projects
at https://github.com/solidity-external-tests/. If changes are needed to make a project work with the
latest version of the compiler, they are maintained as a branch on top of the upstream master branch.
This is especially important for testing our `breaking` branch because we can not realistically expect
external projects to be instantly compatible with a compiler version that has not been released yet.
Applying necessary changes ourselves gives us confidence that breaking changes are sane and that
these projects *can* be upgraded at all.

### Recommended workflow

#### Adding a new external project
1. If the upstream code cannot be compiled without modifications, create a fork of the repository
   in https://github.com/solidity-external-tests/.
2. In our fork, remove all the branches except for main one (`master`, `develop`, `main`, etc).
    This branch is going to be always kept up to date with the upstream repository and should not
    contain any extra commits.
    - If the project is not up to date with the latest compiler version but has a branch that is,
        try to use that branch instead.
3. In our fork, create a new branch named after the main branch and the compiler version from our
 `develop` branch.
     E.g. if the latest Solidity version is 0.7.5 and the main branch of the external project
     is called `master`, create `master_070`. This is where we will be adding our own commits.
4. Create a script for compiling/testing the project and put it in `test/externalTests/` in the
    Solidity repository.
    - The script should apply workarounds necessary to make the project actually use the compiler
        binary it receives as a parameter and possibly add generic workarounds that should
        work across different versions of the upstream project.
    - Very specific workarounds that may easily break with every upstream change are better done as
        commits in the newly added branch in the fork instead.
5. List the script in `test/externalTests.sh`.
6. Add the script to CircleCI configuration. Make sure to add both a compilation-only run and one that
    also executes the test suite. If the latter takes a significant amount of time (say, more than
    15 minutes) make it run nightly rather than on every PR.
7. Make sure that tests pass both on `develop` and on `breaking`. If the compiler from `breaking`
    branch will not work without additional changes, add another branch, called after it in turn,
    and add necessary workarounds there. Continuing the example above, the new branch would be
    called `master_080` and should be rebased on top of `master_070`.
    - The fewer commits in these branches, the better. Ideally, any changes needed to make the compiler
        work should be submitted upstream and our scripts should be using the upstream repository
        directly.

#### Updating external projects for a PR that introduces breaking changes in the compiler
If a PR to our `breaking` branch introduces changes that will make an external project no longer
compile or pass its tests, the fork needs to be modified (or created if it does not yet exist):
- If a branch specific to the compiler version from `breaking` does not exist yet:
    1. Create the branch. It should be based on the version-specific branch used on `develop`.
    2. Make your PR modify the project script in `test/externalScripts/` to use the new branch.
    3. You are free to add any changes you need in the new branch since it will not interfere with
        tests on `breaking`.
    4. Work on your PR until it is approved and merged into `breaking`.
- If the branch already exists and our CI depends on it:
    1. If the external project after your changes can still work with `breaking` even without your PR or
        if you know that the PR is straightforward and will be merged immediately without interfering
        with tests on `breaking` for a significant amount of time, you can just push your modifications
        to the branch directly and skip straight to steps 4. and 6.
    2. Create a PR in the fork, targeting the existing version-specific branch.
    3. In your PR to `breaking`, modify the corresponding script in `test/externalScripts/` to
        use the branch from your PR in the fork.
    4. Work on your PR until it is approved and ready to merge.
    5. Merge the PR in the fork.
    6. Discard your changes to the script and merge your PR into `breaking`.

#### Pulling upstream changes into a fork
1. Pull changes directly into the main branch in the fork. This should be straightforward thanks to
    it not containing any of our customizations.
2. If the project has been updated to a newer Solidity version, abandon the current version-specific
    branch used on `develop` (but do not delete it) and create a new one corresponding to the newer
    version. Then update project script in `test/externalTests/` to use the new branch. E.g. if `develop` uses
    `master_050` and the project has been updated to use Solidity 0.7.3, create `master_070`.
3. Otherwise, rebase the current version-specific branch on the main branch of the fork. This may require
    tweaking some of the commits to apply our fixes in new places.
4. If we have a separate branch for `breaking`, rebase it on top of the one used on `develop`.

The above is the workflow to use when the update is straightforward and looks safe. In that case it is
fine to just modify the branches directly. If this is not the case, it is recommended to first perform the
operation on copies of these version-specific branches and test them by creating PRs on `develop` and
`breaking` to see if tests pass. The PRs should just modify project scripts in `test/externalScripts/`
to use the updated copies of the branches and can be discarded afterwards without being merged.

#### Changes needed after a breaking release of the compiler
When a non-backwards-compatible version becomes the most recent release, `breaking` branch
gets merged into `develop` which automatically results in a switch to the newer version-specific
branches if they exist. If no changes on our part were necessary, it is completely fine to keep using
e.g. the `master_060` of an external project in Solidity 0.8.x.

Since each project is handled separately, this approach may result in a mix of version-specific branches
between different external projects. For example, in one project we could could have `master_050` on
both `develop` and `breaking` and in another `breaking` could use `master_080` while `develop` still
uses `master_060`.
