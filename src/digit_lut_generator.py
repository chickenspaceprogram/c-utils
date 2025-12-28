#!/usr/bin/env python3

def printf(str):
    print(str, end='')

for i in range(16):
    for j in range(16):
        val = i * 16 + j
        printval = 0
        if val >= ord('0') and val <= ord('9'):
            printval = val - ord('0')
        elif val >= ord('a') and val <= ord('z'):
            printval = val - ord('a') + 10;
        elif val >= ord('A') and val <= ord('Z'):
            printval = val - ord('A') + 10;
        else:
            printval = 36
        printf(f"{printval}, ")
    printf("\n")
