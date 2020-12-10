## Solidity external tests
This directory contains scripts for compiling some of the popular open-source projects using the
current version of the compiler and running their test suites.

Since projects often do not use the latest compiler, we keep a fork of each of these projects
at https://github.com/solidity-external-tests/. If changes are needed to make a project work with the
latest version of the compiler, they are maintained as a branch on top of the upstream master branch.
This is especially important for testing our `breaking` branch because we can not realistically expect
external projects to be instantly compatible with a compiler version that has not been released yet.
It also isolates us from upstream changes to some degree - their changes will not affect our test suite
until we explicitly pull them.

### Recommended workflow

#### Adding a new external project
1. Create a fork of the upstream repository in https://github.com/solidity-external-tests/. If the
    project consists of several repositories, fork them all.
2. Remove all the branches except for main one (`master`, `develop`, `main`, etc). This branch is
    going to be always kept up to date with the upstream repository and should not contain any extra
    commits.
3. Create two new versions of the main branch, representing versions of the compiler currently in
    `develop` and `breaking`. E.g. if the latest Solidity version is 0.7.5 and the main branch of the
    external project is called `master`, create `master_070` and `master_080`. This is where we will
    be adding our own commits. The one corresponding to the newer Solidity version should always be
    rebased on top of the older one. E.g. if a change is needed to keep the project
    compilable using Solidity 0.7.x, add it in `master_070` and rebase `master_080` on top of it.
    If it is only needed for the compiler from its `breaking` branch, add it only in `master_080`.
    - The fewer commits in these branches, the better. Ideally, any changes needed to make the compiler
        work should be submitted upstream and the branches should just be tracking the upstream
        one without any extra commits.
4. Create a script for compiling/testing the project and put it in `test/externalTests/` in the
    Solidity repository.
    - The script should apply workarounds necessary to make the project actually use the compiler
      binary it receives as a parameter and possibly apply some generic workarounds that should
      work across different versions of the upstream project. Very specific workarounds that may
      easily break with every upstream change are better done as commits in the fork.
5. Add the script to `tests/externalTests.sh`.
6. Add the test to CircleCI configuration. Make sure to add both a compilation-only run and one that
    also executes the test suite. If the latter takes a significant amount of time (say, more
    than 15 minutes) make it run nightly rather than on every PR.

#### Pulling upstream changes into a fork
1. Pull changes directly into the main branch in the fork. This should be straightforward thanks to
    it not containing any of our customizations.
2. If the update is straightforward and looks safe, go straight to point 5. Otherwise you need to
    test it first.
3. Create new branches corresponding to the two versioned ones but suffixed with `_new`. E.g.
    `master_070_new` and `master_080_new`. Then rebase them so that they are on top of the updated
    main branch. This may require tweaking some of the commits to apply our fixes in new places.
4. Create PRs on `develop` and `breaking` in Solidity repo which modify the script to use the new
    branches instead of `master_070`/`master_080`. Tweak the new branches until external tests
    in CI pass in the PRs.
5. Discard the PRs and move the original branches to the place where the `_new` ones are and remove
    the `_new` branches.

#### Changes needed after a breaking release of the compiler
When a breaking version of the compiler gets released and becomes the most recent release, the scripts
and the branches in the forks need to be updated:
- In each fork create a branch corresponding to the new breaking version. E.g. if the current
    branches are `master_070` and `master_080` the new one should be called `master_090`.
- Leave the oldest (i.e. `master_070`) branch as is. We will not be updating it any more but there is no
    need to remove it.
- Update scripts on `develop` to now refer to the branch that used to be breaking (i.e. `master_080`)
    and on `breaking` to refer to the newly added branch (i.e. `master_090`).
    - Take care not to overwrite these changes when merging `develop` into breaking. Each branch
        should always use the branches from the external repos that correspond to it.
