#!/usr/bin/env python3

import sys
import random

def main(argv):
    az = "abcdefghijklmnopqrstuvwxyz"
    assert len(az) == 26
    assert len(set(az)) == len(az)
    row_width = 50
    number_of_rows = 30

    letters = []
    for _ in range(number_of_rows):
        letters.append("".join(random.choice(az) for _ in range(row_width)))
    print("\n".join(letters))

if __name__ == "__main__":
    sys.exit(main(sys.argv))
