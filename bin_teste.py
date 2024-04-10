#!/usr/bin/python3
bits = [1, 1, 1]
total = len(bits)

for i in range((2**total)-2, -1, -1):
    result = []
    for j in range(total):
        result.append(bits[j] & i>>(total-j-1))

    print(f"{bin(i):5} | {bits} | {result}")