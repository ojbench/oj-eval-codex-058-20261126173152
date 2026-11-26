# Pokedex solution

`src.hpp` implements the judge interface. An ordered map stores records by ID;
seven-bit masks represent their types. The constructor loads records and the
destructor writes them back. Argument validation precedes all mutations, and
iterators check boundary operations and empty dereferences.

Capture attempts group records by type mask. Each acquired attack type is
processed once, adding every vulnerable group and making its types available.
This computes the same closure as individual capture attempts.

Complexity for n records:
- Add, delete, and ID lookup: O(log n).
- Type lookup: O(n + output size).
- Capture: O(n + 7 * 128 * 7), with constant auxiliary space.
- Storage: O(n).

## Verification

Run local checks with:

```sh
g++ -std=c++14 -Wall -Wextra -Werror -fsanitize=address,undefined -g tests/test.cpp -o /tmp/pokedex-test
/tmp/pokedex-test
```

Checks cover persistence, exceptions, iterator boundaries, damage, and 300
randomized capture comparisons against an individual-Pokemon simulation.
The README's sample-data directory was unavailable.

## Judge results

1. Submission 879503: Compile Error. The supplied Git submission mode caused
   the judge to compile the repository URL as header contents.
2. Submission 879507: Accepted, 600/600 points, all 20 test points passed.
   The client was extended with `--file` to submit `src.hpp` contents directly.
   Reported total time: 69 ms; peak memory: 4,505,600 bytes.

Both submissions followed a verified Git push. No repository changes or pushes
were made while either submission was being evaluated.
