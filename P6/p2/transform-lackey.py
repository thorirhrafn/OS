#!/usr/bin/env python3
from asyncio.subprocess import DEVNULL
import os
import subprocess

# Put your script here (instead of the print statements)
# Your script shall read the valgrind output on stdin,
# and produce a list of newline-separated lines of virtual page numbers
# valgrind --tool=lackey --trace-mem=yes ls -la 2>&1 1>/dev/null
#output = os.system("valgrind --tool=lackey --trace-mem=yes ls -la")
page_numbers = []

valgrind_output = subprocess.run(["valgrind", "--tool=lackey", "--trace-mem=yes", "ls", "-la"], stdout=DEVNULL, stderr=subprocess.PIPE)

address_list = valgrind_output.stderr.split()
for item in address_list:
    first_char = item[0]
    if 47 < first_char < 58:
        size = len(item)
        if size > 5:
            new_item = str(item).split(",")
            for num in new_item:
                if len(num) > 5:
                    #print("---hex item---")
                    #print(item)
                    #print(num)
                    num_trunc = num[2:size-3]
                    #print("---offsett removed---")
                    #print(num_trunc)
                    try:
                        page_num = int(str(num_trunc),16)
                        #print("---converted to decimal---")
                        #print(page_num)
                        page_numbers.append(page_num)
                    except ValueError:
                        break    


textfile = open("page_numbers.txt", "w")
for number in page_numbers:
    textfile.write(str(number) + ",")
textfile.close()

#for item in address_list.stderr:
#    print(item)

#print(42)
#print(4711)
#print(12345)
