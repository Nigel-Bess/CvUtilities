'''
Use this python script to convert JSON files downloaded from Intel Realsense viewer or their website into a format that
can be accepted by device_presets.cpp

Steps:
1. Copy + paste only the BODY of the json (no brackets) into a file of your choice, and unindent so no blankspace
preceeding each line
2. Run this script, a converted version will be generated in a file called converted_file.json
3. Copy+paste contents into desired section of device_presets.cpp, making sure to edit brackets and the last line of
content manually.
'''


import sys

file = sys.argv[1]

f = open(file, "r")
out = open("converted_file.json", "w")

for line in f:
    new_line = '"' + '    ' + '\\' + line.replace('": ', r'\":' + '\\').replace('",', r'\",\n"')
    out.write(new_line)

f.close()
out.close()
